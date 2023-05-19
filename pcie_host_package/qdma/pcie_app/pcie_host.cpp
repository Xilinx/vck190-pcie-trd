/*
 * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

#include "pcie_host.h"
#include "mainwindow.h"
#define _BSD_SOURCE
//#define _XOPEN_SOURCE 500
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "dma_xfer_utils.c"
#include <glib-2.0/glib.h>

#define MAP_SIZE (32*1024UL)
#define MAP_MASK (MAP_SIZE - 1)
#include<signal.h>
// 
#define PCIRC_GET_FILE_LENGTH      		0x04
#define PCIRC_PID_HIGH				0x0C
#define PCIRC_FILTER_PARAMS        		0x14
#define PCIRC_RAW_RESOLUTION      		0x18
#define PCIRC_UCASE_SET          		0x20
#define PCIRC_FILTER_MODE          		0x24
#define PCIRC_FPS_SET          			0x28
#define PCIEP_SET_SIG                           0x34
#define PCIRC_FMT_SET         			0x3c

//  Interrupts
#define PCIRC_READ_BUFFER_TRANSFER_DONE 	0x70
#define PCIRC_WRITE_BUFFER_TRANSFER_DONE 	0x74
#define PCIRC_HOST_DONE			  	0x78

#define KGREEN  "\x1B[32m"
#define KRED "\x1B[31m"
#define RESET "\x1B[0m"

//#define T_DEBUG
#define FAILURE -1

//Planes Multiplier
#define YUV_MULTIPLIER 2
#define BGR_MULTIPLIER 3

#define PCIEP_READ_BUFFER_READY   		0x80
#define PCIEP_READ_BUFFER_ADDR_LOW   		0x84
#define PCIEP_READ_BUFFER_OFFSET 		0x88
#define PCIEP_READ_BUFFER_SIZE   		0x8c

#define PCIEP_WRITE_BUFFER_READY   		0x90
#define PCIEP_WRITE_BUFFER_ADDR_LOW   		0x94
#define PCIEP_WRITE_BUFFER_OFFSET 		0x98
#define PCIEP_WRITE_BUFFER_SIZE   		0x9c

#define PCIEP_READ_TRANSFER_COMPLETE   		0xa0
#define PCIEP_WRITE_TRANSFER_COMPLETE  		0xa4
#define PCIEP_WRITE_BUFFER_ADDR_HIGH   		0xb8
#define PCIEP_READ_BUFFER_ADDR_HIGH   		0xbc


#define H2C_DEVICE "/dev/qdma03000-MM-0"
#define C2H_DEVICE "/dev/qdma03000-MM-1"
#define REG_DEVICE_NAME "/sys/bus/pci/devices/0000:03:00.0/resource0"
#define COUNT_DEFAULT 		  (1)
#define SIZE_DEFAULT		  (32)
/* Frame rate options */
#define FPS_MIN               (30)
#define FPS_MAX               (60)
#define FPS_DEFAULT           (FPS_MIN)
#define FMT_DEFAULT           (0)
#define PID_DEFAULT           42
#define UCASE_DEFAULT          0 

#define FILTER_TYPE_DEFAULT         (1)
#define FILTER_MODE_DEFAULT (1)
/* Input file params */
#define INPUT_WIDTH_DEFAULT        (1920)
#define INPUT_HEIGHT_DEFAULT       (1080)
typedef enum
{
	FILTER_BLUR,
	FILTER_EDGE,
	FILTER_HEDGE,
	FILTER_VEDGE,
	FILTER_EMBOSS,
	FILTER_HGRAD,
	FILTER_VGRAD,
	FILTER_IDENTITY,
	FILTER_SHARPEN,
	FILTER_HSOBEL,
	FILTER_VSOBEL,
} filter_type;

typedef enum
{
	SW,
	HW,

}kernel_mode;

struct pcie_transfer {
	int infile_fd;
	int h2c_fd;
	int c2h_fd;
	int reg_fd;
	char *map_base;
	char *read_buffer;
	char *write_buffer;
	char *infname;
} trans;

#define BILLION  1000000000.0

int usecase_sel;

volatile unsigned int *host_done;
unsigned int filter_type_val = FILTER_TYPE_DEFAULT;
unsigned int filter_mode = FILTER_MODE_DEFAULT; 
unsigned int in_width = INPUT_WIDTH_DEFAULT;
unsigned int in_height = INPUT_HEIGHT_DEFAULT;
unsigned int fps = FPS_DEFAULT;
unsigned int u_case = UCASE_DEFAULT;
volatile unsigned int *set_sig;

circular_buffer queue_frame;
circular_buffer queue_file_frame;
bool app_running = true;

GIOChannel* io_stdin = NULL;
guint  add_watch;

int flag = 0;

	static gboolean
handle_keyboard (GIOChannel *source, GIOCondition , gpointer *)
{
	gchar    *str   = NULL;
	gboolean ret    = TRUE;

	if (g_io_channel_read_line (source, &str, NULL, NULL, NULL) != G_IO_STATUS_NORMAL) {
		return FALSE;
	}
	if (g_ascii_tolower (str[0]) == 'q') {
		printf("Quitting the MIPI Usecase \n");
		set_sig = ((uint32_t *)(trans.map_base + PCIEP_SET_SIG));
		*set_sig = 0x1;
		g_source_remove(add_watch);
		g_io_channel_unref (io_stdin);
		flag = 0;
	}	
	g_free (str);

	return ret;
}

int cb_init(circular_buffer *cb, size_t capacity, size_t sz)
{
	cb->buffer = (char *)malloc(capacity * sz);
	if (cb->buffer == NULL) {
		printf("Error unable to allocate buffers\n");
		return -ENOMEM;
	}

	cb->buffer_end = (char *)cb->buffer + (capacity * sz);
	cb->capacity = capacity;
	cb->index = 0;
	cb->sz = sz;
	cb->head = cb->buffer;
	cb->tail = cb->buffer;

	return 0;
}

int cb_enque(circular_buffer *cb, char *data)
{
	if(cb->index == cb->capacity)
		return 1;

	memcpy(cb->head, data, cb->sz);
	cb->head = cb->head + cb->sz;
	if (cb->head == cb->buffer_end)
		cb->head = cb->buffer;
	cb->index++;
	return 0;
}

int cb_deque(circular_buffer *cb, char *data)
{
	if (cb->index == 0) 
		return 1;

	memcpy(data, cb->tail, cb->sz);
	cb->tail = (char *)cb->tail + cb->sz;
	if (cb->tail == cb->buffer_end)
		cb->tail = cb->buffer;
	cb->index--;
	return 0;
}

typedef enum {
    VGST_USECASE_TYPE_NONE = 0,
    VGST_USECASE_TYPE_MIPISRC_TO_HOST,
    VGST_USECASE_TYPE_MIPISRC_DPU_TO_HOST,
    VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS,
    VGST_USECASE_TYPE_APPSRC_TO_HOST,
    VGST_USECASE_TYPE_APPSRC_TO_HOST_BYPASS,
    VGST_USECASE_TYPE_APPSRC_TO_KMSSINK,
    VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK,
    VGST_USECASE_TYPE_APPSRC_TO_KMSSINK_BYPASS,
    VGST_USECASE_TYPE_MAX,
} VGST_USECASE_TYPE;

int cmaincall(struct MainWindow *frm)
{
	const char *h2c_device = H2C_DEVICE;
	const char *c2h_device = C2H_DEVICE;
	int ret;

	while(1) {
		usecase_sel = 0;
		printf("Enter %d to run  : MIPI-->filter2d-->pciesink--> displayonhost\n",VGST_USECASE_TYPE_MIPISRC_TO_HOST);
		printf("Enter %d to run  : MIPI-->dpu-->pciesink--> displayonhost\n",VGST_USECASE_TYPE_MIPISRC_DPU_TO_HOST);
		printf("Enter %d to run  : MIPI-->pciesink--> displayonhost\n",VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS);
		printf("Enter %d to run  : RawVideofilefromHost-->pciesrc-->filter2d-->pciesink-->displayonhost\n",VGST_USECASE_TYPE_APPSRC_TO_HOST);
		printf("Enter %d to run  : RawVideofilefromHost--> pciesrc-->pciesink-->displayonhost\n",VGST_USECASE_TYPE_APPSRC_TO_HOST_BYPASS);
		printf("Enter %d to run  : RawVideofilefromHost--> pciesrc-->filter2d-->kmssink\n",VGST_USECASE_TYPE_APPSRC_TO_KMSSINK);
		printf("Enter %d to run  : RawVideofilefromHost--> pciesrc-->dpu-->kmssink\n",VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK);
		printf("Enter %d to run  : RawVideofilefromHost--> pciesrc-->kmssink\n",VGST_USECASE_TYPE_APPSRC_TO_KMSSINK_BYPASS);
		printf("Enter %d to 	: Exit application\n",VGST_USECASE_TYPE_MAX);
		
		printf("Enter your choice:");
		scanf("%d",&usecase_sel);
		switch(usecase_sel)
		{
			case VGST_USECASE_TYPE_MIPISRC_TO_HOST: 
				ret = mipi_displayonhost(frm,c2h_device);
				if (ret < 0) {
					printf("please run pipeline correctly \n");
					frm->getVidFrame0()->ctrlc(); 
					return 0;	
				}	
				break; 	
			case VGST_USECASE_TYPE_MIPISRC_DPU_TO_HOST:
			case VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS: 
				ret = mipi_displayonhost(frm,c2h_device);
				if (ret < 0) {
					printf("please run pipeline correctly \n");
					frm->getVidFrame0()->ctrlc(); 
					return 0;	
				}	
				break; 	
			case VGST_USECASE_TYPE_APPSRC_TO_HOST: 
				ret = host2host(frm,h2c_device,c2h_device);
				if (ret < 0) {
					printf("please run pipeline correctly \n");	
					frm->getVidFrame0()->ctrlc(); 
					return 0;	
				} 
				break; 	
			case VGST_USECASE_TYPE_APPSRC_TO_HOST_BYPASS: 
				ret = host2host_without_filter(frm,h2c_device,c2h_device);
				if (ret < 0) {
					printf("please run pipeline correctly \n");	
					frm->getVidFrame0()->ctrlc(); 
					return 0;	
				}
				break;
			case VGST_USECASE_TYPE_APPSRC_TO_KMSSINK:
				ret = host2kmssink_with_filter(c2h_device);
                                if (ret < 0) {
                                        printf("please run pipeline correctly \n");
                                        frm->getVidFrame0()->ctrlc();
                                        return 0;
                                }
				break;
			case VGST_USECASE_TYPE_APPSRC_TO_KMSSINK_BYPASS:
			case VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK:
				ret = host2kmssink_with_filter(c2h_device);
                                if (ret < 0) {
                                        printf("please run pipeline correctly \n");
                                        frm->getVidFrame0()->ctrlc();
                                        return 0;
                                }
				break;	
			case VGST_USECASE_TYPE_MAX:	
				frm->getVidFrame0()->ctrlc(); 
				return 0;	
			default :
				printf("Enter choice 1-%d\n", VGST_USECASE_TYPE_MAX);
				break; 	
		}	 


	}
}

int mipi_displayonhost(struct MainWindow *frm,const char *c2h_device)
{
	int rc;
	filter_type fil_type;
	kernel_mode ker_mode;
	volatile unsigned int *input_res = NULL;
	volatile unsigned int *filter_params;
	volatile unsigned int *kernel_mode_params;
	volatile unsigned int *ucase_params;
	volatile unsigned int *fps_mode_params;
	int choice;
	pthread_t  thread3;	

	printf("select the resolution \n");
	printf("1. 3840x2160\n");
	printf("2. 1920x1080\n");

	printf("Enter your choice:");
	scanf("%d",&choice);
	if (choice == 1) {
		in_width  = 3840;
		in_height = 2160;
	}
	else {
		in_height = 1080;
		in_width  = 1920;
	}
	app_running = true;
	rc = 0;

	frm->getVidFrame0()->setResolution(in_width,in_height,30);
	frm->getVidFrame0()->config_frame();

	trans.c2h_fd = open(c2h_device, O_RDWR);
	if (trans.c2h_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
				c2h_device, trans.c2h_fd);
		rc = -EINVAL;
		goto out;
	}
	trans.reg_fd = open(REG_DEVICE_NAME, O_RDWR);
	if (trans.reg_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
				REG_DEVICE_NAME, trans.reg_fd);
		perror("open reg device failed");
		rc = -EINVAL;
		goto c2h_out;
	}
	
	trans.map_base = (char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, trans.reg_fd, 0);
	if (trans.map_base == (void *)-1) {
		printf("Memory mapped at address %p.\n", trans.map_base);
		fflush(stdout);
		rc = -ENOMEM;
		goto reg_fd;

	}
	if(usecase_sel == 1){
		printf("Enter filter type value 0-10:");
		scanf("%d",&filter_type_val);
	}
	u_case = usecase_sel;
	ucase_params = ((uint32_t *)(trans.map_base + PCIRC_UCASE_SET));
	*ucase_params = u_case & 0xFFFFFFFF;

	filter_mode = 1;
	ker_mode = (kernel_mode)filter_mode;
	kernel_mode_params = ((uint32_t *)(trans.map_base + PCIRC_FILTER_MODE));
	*kernel_mode_params = ker_mode;

	fps = 30;
	fps_mode_params = ((uint32_t *)(trans.map_base + PCIRC_FPS_SET));
	*fps_mode_params = fps & 0xFFFFFFFF;


	input_res = ((uint32_t *)(trans.map_base + PCIRC_RAW_RESOLUTION));
	*input_res = (in_height << 16) | in_width;
	
	if(usecase_sel == 1){
		fil_type = (filter_type)filter_type_val;
		filter_params = ((uint32_t *)(trans.map_base + PCIRC_FILTER_PARAMS));
		*filter_params = fil_type;
	}

	printf(KGREEN "\nPlease run 'vck190-pcie-trd-nb1.ipynb' jupyter notebook from endpoint (To launch endpoint application)\n" RESET); 
	printf(KRED "To quit usecase, hit <q+enter> from host \n\n" RESET);
	
	io_stdin = g_io_channel_unix_new (fileno (stdin));
	add_watch = g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc)handle_keyboard, NULL);
	flag = 1;
	if(usecase_sel == 2 || usecase_sel == 7 )
	rc = cb_init(&queue_frame, 240, in_width * in_height * BGR_MULTIPLIER);
	else
	rc = cb_init(&queue_frame, 240, in_width * in_height * YUV_MULTIPLIER);
	if (rc < 0)
		goto reg_unmap;

	if(usecase_sel == 2 || usecase_sel == 7 )
	rc = cb_init(&queue_file_frame, 120, in_width * in_height * BGR_MULTIPLIER);
	else
	rc = cb_init(&queue_file_frame, 120, in_width * in_height * YUV_MULTIPLIER);
	if (rc < 0)
		goto free_qframe;

	pthread_create( &thread3, NULL, &pcie_dma_write, NULL);

	pthread_join( thread3, NULL);

	app_running = false;
	while(queue_frame.index);
	host_done = ((uint32_t *)(trans.map_base + PCIRC_HOST_DONE));
	*host_done = 0x1;
	set_sig = ((uint32_t *)(trans.map_base + PCIEP_SET_SIG));
	*set_sig = 0x0;

	if (flag == 1) {	
		g_source_remove(add_watch);
		g_io_channel_unref (io_stdin);
	}

free_qframe:
	free(queue_frame.buffer);
reg_unmap:
	if (munmap(trans.map_base, MAP_SIZE) == -1)
		printf("error unmap\n");
reg_fd :	
	if (trans.reg_fd >= 0)
		close(trans.reg_fd);
c2h_out:
	close(trans.c2h_fd);
out:
	app_running = false;
	return rc;
}
int host2kmssink_with_filter(const char *h2c_device)
{
        ssize_t rc;
        volatile unsigned int *host_done;
        volatile unsigned int *infile_len;
        unsigned long int file_len;
        filter_type fil_type;
        kernel_mode ker_mode;
        volatile unsigned int *input_res = NULL;
        volatile unsigned int *filter_params;
        volatile unsigned int *kernel_mode_params;
        volatile unsigned int *fps_mode_params;
        char infilename[100];
        int choice;
        volatile unsigned int *ucase_params;

        pthread_t thread1, thread2;

        printf("select the resolution \n");
        printf("1. 3840x2160\n");
        printf("2. 1920x1080\n");

        printf("Enter your choice:");
        scanf("%d",&choice);
        if (choice == 1) {
                in_width  = 3840;
                in_height = 2160;
        }
        else {
                in_height = 1080;
                in_width  = 1920;
        }

        trans.h2c_fd = open(h2c_device, O_RDWR);
        if (trans.h2c_fd < 0) {
                fprintf(stderr, "unable to open device %s, %d.\n",
                                h2c_device, trans.h2c_fd);
                app_running = false;
                return -EINVAL;
        }

        trans.reg_fd = open(REG_DEVICE_NAME, O_RDWR);
        if (trans.reg_fd < 0) {
                fprintf(stderr, "unable to open device %s, %d.\n",
                                REG_DEVICE_NAME, trans.reg_fd);
                rc = -EINVAL;
                goto h2c_out;
        }

        /* map one page */
        trans.map_base = (char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, trans.reg_fd, 0);
        if (trans.map_base == (void *)-1) {
                printf("Memory mapped at address %p.\n", trans.map_base);
                fflush(stdout);
                rc = -EINVAL;
                goto reg_fd;

        }
        printf("Enter input filename with path to transfer:");
        scanf("%s",infilename);

        trans.infname = infilename;
        trans.infile_fd = open(infilename, O_RDONLY);
        if (trans.infile_fd < 0) {
                fprintf(stderr, "unable to open input file %s, %d.\n",
                                infilename, trans.infile_fd);
                rc = -EINVAL;
                goto reg_unmap;
        }
        if(usecase_sel == 6){
		printf("Enter Filter type value 0-10:");
        	scanf("%d",&filter_type_val);

        /* setting kernel mode */
        filter_mode = 1;
        ker_mode = (kernel_mode)filter_mode;
        kernel_mode_params = ((uint32_t *)(trans.map_base + PCIRC_FILTER_MODE));
        *kernel_mode_params = ker_mode;
	}

        fps = 30;
       	fps_mode_params = ((uint32_t *)(trans.map_base + PCIRC_FPS_SET));
        *fps_mode_params = fps & 0xFFFFFFFF;

        /* Get the input file length  */
        if (trans.infile_fd > 0) {
                file_len = lseek(trans.infile_fd, 0, SEEK_END);
                if (file_len <= 0)
                {
                        printf("failed to lseek %s numbytes %lu\n", trans.infname, file_len);
                        rc = -EINVAL;
                        goto infile_out;
                }
                /* reset the file position indicator to
                   the beginning of the file */
                lseek(trans.infile_fd, 0L, SEEK_SET);
                infile_len = ((uint32_t *)(trans.map_base + PCIRC_GET_FILE_LENGTH));
                *infile_len = (unsigned int)(file_len & 0xFFFFFFFF);
                infile_len = ((uint32_t *)(trans.map_base + PCIRC_GET_FILE_LENGTH - 4));
                *infile_len = (unsigned int)(file_len >> 32 & 0xFFFFFFFF);
        }
        /* setting input resolution */
        input_res = ((uint32_t *)(trans.map_base + PCIRC_RAW_RESOLUTION));
        *input_res = (in_height << 16) | in_width;
	
	if(usecase_sel == 6) {
        	fil_type = (filter_type)filter_type_val;
        	filter_params = ((uint32_t *)(trans.map_base + PCIRC_FILTER_PARAMS));
        	*filter_params = fil_type;
	}
        u_case = usecase_sel;
        ucase_params = ((uint32_t *)(trans.map_base + PCIRC_UCASE_SET));
        *ucase_params = u_case & 0xFFFFFFFF;

        printf(KGREEN "\nPlease run 'vck190-pcie-trd-nb1.ipynb' jupyter notebook from endpoint (To launch endpoint application)\n\n" RESET);
        
	if(usecase_sel == 2 || usecase_sel == 7)
		rc = cb_init(&queue_frame, 240, in_width * in_height * BGR_MULTIPLIER);
	else
		rc = cb_init(&queue_frame, 240, in_width * in_height * YUV_MULTIPLIER);
        if (rc < 0)
                goto infile_out;

	if(usecase_sel == 2 || usecase_sel == 7)
        	rc = cb_init(&queue_file_frame, 120, in_width * in_height * BGR_MULTIPLIER);
        else
		rc = cb_init(&queue_file_frame, 120, in_width * in_height * YUV_MULTIPLIER);
        if (rc < 0)
                goto free_qframe;

        pthread_create( &thread2, NULL, &file_read, NULL);
        pthread_create( &thread1, NULL, &pcie_dma_read, NULL);


        pthread_join( thread1, NULL);
        pthread_join( thread2, NULL);

        while(queue_frame.index);
        host_done = ((uint32_t *)(trans.map_base + PCIRC_HOST_DONE));
        *host_done = 0x1;


        *infile_len = 0x0;
        free(queue_file_frame.buffer);

free_qframe:
        free(queue_frame.buffer);
infile_out:
        if (trans.infile_fd >= 0)
                close(trans.infile_fd);
reg_unmap:
        if (munmap(trans.map_base, MAP_SIZE) == -1)
                printf("error unmap\n");
reg_fd:
        if (trans.reg_fd >= 0)
                close(trans.reg_fd);
h2c_out:
        if (trans.h2c_fd >= 0)
                close(trans.h2c_fd);
        return rc;
}


int host2host(struct MainWindow *frm,const char *h2c_device,const char *c2h_device)
{
	ssize_t rc;
	volatile unsigned int *host_done;
	volatile unsigned int *infile_len;
	unsigned long int file_len;
	filter_type fil_type;
	kernel_mode ker_mode;
	volatile unsigned int *input_res = NULL;
	volatile unsigned int *filter_params;
	volatile unsigned int *kernel_mode_params;
	volatile unsigned int *fps_mode_params;
	char infilename[100];
	int choice;
	volatile unsigned int *ucase_params;

	pthread_t thread1, thread2, thread3;

	printf("select the resolution \n");
	printf("1. 3840x2160\n");
	printf("2. 1920x1080\n");

	printf("Enter your choice:");
	scanf("%d",&choice);

	if (choice == 1) {
		in_width  = 3840;
		in_height = 2160;
	}
	else {
		in_height = 1080;
		in_width  = 1920;
	}

	app_running = true;

	frm->getVidFrame0()->setResolution(in_width,in_height,30);
	frm->getVidFrame0()->config_frame();

	trans.h2c_fd = open(h2c_device, O_RDWR);
	if (trans.h2c_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
				h2c_device, trans.h2c_fd);
		app_running = false;
		return -EINVAL;
	}

	trans.c2h_fd = open(c2h_device, O_RDWR);
	if (trans.c2h_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
				c2h_device, trans.c2h_fd);
		rc = -EINVAL;
		goto h2c_out;
	}

	trans.reg_fd = open(REG_DEVICE_NAME, O_RDWR);
	if (trans.reg_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
				REG_DEVICE_NAME, trans.reg_fd);
		rc = -EINVAL;
		goto c2h_out;
	}

	/* map one page */
	trans.map_base = (char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, trans.reg_fd, 0);
	if (trans.map_base == (void *)-1) {
		printf("Memory mapped at address %p.\n", trans.map_base);
		fflush(stdout);
		rc = -EINVAL;
		goto reg_fd;

	}
	printf("Enter input filename with path to transfer:");
	scanf("%s",infilename);

	trans.infname = infilename;
	trans.infile_fd = open(infilename, O_RDONLY);
	if (trans.infile_fd < 0) {
		fprintf(stderr, "unable to open input file %s, %d.\n",
				infilename, trans.infile_fd);
		rc = -EINVAL;
		goto reg_unmap;
	}
	printf("Enter Filter type value 0-10:");
	scanf("%d",&filter_type_val);


	/* setting kernel mode */
	filter_mode = 1;
	ker_mode = (kernel_mode)filter_mode;
	kernel_mode_params = ((uint32_t *)(trans.map_base + PCIRC_FILTER_MODE));
	*kernel_mode_params = ker_mode;

	fps = 30;
	fps_mode_params = ((uint32_t *)(trans.map_base + PCIRC_FPS_SET));
	*fps_mode_params = fps & 0xFFFFFFFF;

	/* Get the input file length  */
	if (trans.infile_fd > 0) {
		file_len = lseek(trans.infile_fd, 0, SEEK_END);
		if (file_len <= 0)
		{
			printf("failed to lseek %s numbytes %lu\n", trans.infname, file_len);
			rc = -EINVAL;
			goto infile_out;
		}
		/* reset the file position indicator to
		   the beginning of the file */
		lseek(trans.infile_fd, 0L, SEEK_SET);
		infile_len = ((uint32_t *)(trans.map_base + PCIRC_GET_FILE_LENGTH));
		*infile_len = (unsigned int)(file_len & 0xFFFFFFFF);
		infile_len = ((uint32_t *)(trans.map_base + PCIRC_GET_FILE_LENGTH - 4));
		*infile_len = (unsigned int)(file_len >> 32 & 0xFFFFFFFF);
	}
	/* setting input resolution */
	input_res = ((uint32_t *)(trans.map_base + PCIRC_RAW_RESOLUTION));
	*input_res = (in_height << 16) | in_width;

	fil_type = (filter_type)filter_type_val;
	filter_params = ((uint32_t *)(trans.map_base + PCIRC_FILTER_PARAMS));
	*filter_params = fil_type;

	u_case = usecase_sel;
	ucase_params = ((uint32_t *)(trans.map_base + PCIRC_UCASE_SET));
	*ucase_params = u_case & 0xFFFFFFFF;

	printf(KGREEN "\nPlease run 'vck190-pcie-trd-nb1.ipynb' jupyter notebook from endpoint (To launch endpoint application)\n\n" RESET);
	
	if(usecase_sel == 2 || usecase_sel == 7)
		rc = cb_init(&queue_frame, 240, in_width * in_height * BGR_MULTIPLIER);
	else
		rc = cb_init(&queue_frame, 240, in_width * in_height * YUV_MULTIPLIER);
	if (rc < 0)
		goto infile_out;

	if(usecase_sel == 2 || usecase_sel == 7)
		rc = cb_init(&queue_file_frame, 120, in_width * in_height * BGR_MULTIPLIER);
	else
		rc = cb_init(&queue_file_frame, 120, in_width * in_height * YUV_MULTIPLIER);
	if (rc < 0)
		goto free_qframe;

	pthread_create( &thread2, NULL, &file_read, NULL);
	pthread_create( &thread1, NULL, &pcie_dma_read, NULL);
	pthread_create( &thread3, NULL, &pcie_dma_write, NULL);

	pthread_join( thread3, NULL);
	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);

	while(queue_frame.index);
	host_done = ((uint32_t *)(trans.map_base + PCIRC_HOST_DONE));
	*host_done = 0x1;


	*infile_len = 0x0;
	free(queue_file_frame.buffer);
free_qframe:
	free(queue_frame.buffer);
infile_out:
	if (trans.infile_fd >= 0)
		close(trans.infile_fd);
reg_unmap:
	if (munmap(trans.map_base, MAP_SIZE) == -1)
		printf("error unmap\n");
reg_fd:
	if (trans.reg_fd >= 0)
		close(trans.reg_fd);
c2h_out:
	if (trans.c2h_fd >= 0)
		close(trans.c2h_fd);
h2c_out:
	if (trans.h2c_fd >= 0)
		close(trans.h2c_fd);
	app_running = false;
	return rc;
}

int host2host_without_filter(struct MainWindow *frm, const char *h2c_device, const char *c2h_device)
{
	ssize_t rc;
	volatile unsigned int *host_done;
	volatile unsigned int *infile_len;
	unsigned long int file_len;
	kernel_mode ker_mode;
	volatile unsigned int *input_res = NULL;
	volatile unsigned int *kernel_mode_params;
	volatile unsigned int *fps_mode_params;
	char infilename[100];
	int choice;
	volatile unsigned int *ucase_params;
	pthread_t thread1, thread2, thread3;

	printf("select the resolution \n");
	printf("1. 3840x2160\n");
	printf("2. 1920x1080\n");
	printf("Enter Your Choice:");
	scanf("%d",&choice);
	if( choice == 1){
		in_width  = 3840;
		in_height = 2160;
	}
	else {
		in_height = 1080;
		in_width  = 1920;
	}

	app_running = true;
	frm->getVidFrame0()->setResolution(in_width,in_height,30);
	frm->getVidFrame0()->config_frame();

	trans.h2c_fd = open(h2c_device, O_RDWR);
	if (trans.h2c_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
				h2c_device, trans.h2c_fd);
		app_running = false;
		return -EINVAL;
	}

	trans.c2h_fd = open(c2h_device, O_RDWR);
	if (trans.c2h_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
				c2h_device, trans.c2h_fd);
		rc = -EINVAL;
		goto h2c_out;
	}

	trans.reg_fd = open(REG_DEVICE_NAME, O_RDWR);
	if (trans.reg_fd < 0) {

		fprintf(stderr, "unable to open device %s, %d.\n",
				REG_DEVICE_NAME, trans.reg_fd);
		perror("open reg device failed");
		rc = -EINVAL;
		goto c2h_out;
	}

	/* map one page */
	trans.map_base = (char *) mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, trans.reg_fd, 0);
	if (trans.map_base == (void *)-1) {
		printf("Memory mapped at address %p.\n", trans.map_base);
		fflush(stdout);
		rc = -ENOMEM;
		goto reg_fd;

	}

	printf("Enter input filename with path to transfer:");
	scanf("%s",infilename);
	trans.infname = infilename;
	trans.infile_fd = open(infilename, O_RDONLY);
	if (trans.infile_fd < 0) {
		fprintf(stderr, "unable to open input file %s, %d.\n",
				infilename, trans.infile_fd);
		rc = -EINVAL;
		goto reg_unmap;
	}

	filter_mode = 1;
	ker_mode = (kernel_mode)filter_mode;
	kernel_mode_params = ((uint32_t *)(trans.map_base + PCIRC_FILTER_MODE));
	*kernel_mode_params = ker_mode;

	fps = 30;
	fps_mode_params = ((uint32_t *)(trans.map_base + PCIRC_FPS_SET));
	*fps_mode_params = fps & 0xFFFFFFFF;


	/* Get the input file length  */
	if (trans.infile_fd > 0) {
		file_len = lseek(trans.infile_fd, 0, SEEK_END);
		if (file_len <= 0)
		{
			printf("failed to lseek %s numbytes %lu\n", trans.infname, file_len);
			goto infile_out;
		}
		/* reset the file position indicator to
		the beginning of the file */
		lseek(trans.infile_fd, 0L, SEEK_SET);
		infile_len = ((uint32_t *)(trans.map_base + PCIRC_GET_FILE_LENGTH));
		*infile_len = (unsigned int)(file_len & 0xFFFFFFFF);
		infile_len = ((uint32_t *)(trans.map_base + PCIRC_GET_FILE_LENGTH - 4));
		*infile_len = (unsigned int)(file_len >> 32 & 0xFFFFFFFF);
	}

	/* setting input resolution */
	input_res = ((uint32_t *)(trans.map_base + PCIRC_RAW_RESOLUTION));
	*input_res = (in_height << 16) | in_width;

	/* setting Usecase type */
	u_case = usecase_sel;
	ucase_params = ((uint32_t *)(trans.map_base + PCIRC_UCASE_SET));
	*ucase_params = u_case & 0xFFFFFFFF;

	printf(KGREEN "\nPlease run 'vck190-pcie-trd-nb1.ipynb' jupyter notebook from endpoint (To launch endpoint application)\n\n" RESET);
	
	if(usecase_sel == 2 || usecase_sel == 7)	
		rc = cb_init(&queue_frame, 240, in_width * in_height * BGR_MULTIPLIER);
	else
		rc = cb_init(&queue_frame, 240, in_width * in_height * YUV_MULTIPLIER);
	if (rc < 0)
		goto reg_unmap;

	if(usecase_sel == 2 || usecase_sel == 7)	
		rc = cb_init(&queue_file_frame, 120, in_width * in_height * BGR_MULTIPLIER);
	else
		rc = cb_init(&queue_file_frame, 120, in_width * in_height * YUV_MULTIPLIER);
	if (rc < 0)
		goto free_qframe;

	pthread_create( &thread2, NULL, &file_read, NULL);
	pthread_create( &thread1, NULL, &pcie_dma_read, NULL);
	pthread_create( &thread3, NULL, &pcie_dma_write, NULL);

	pthread_join( thread3, NULL);
	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);

	while(queue_frame.index);
	host_done = ((uint32_t *)(trans.map_base + PCIRC_HOST_DONE));
	*host_done = 0x1;

	*infile_len = 0x0;
	free(queue_file_frame.buffer);

free_qframe:
	free(queue_frame.buffer);
infile_out:
	if (trans.infile_fd >= 0)
		close(trans.infile_fd);
reg_unmap:
	if (munmap(trans.map_base, MAP_SIZE) == -1)
		printf("error unmap\n");
reg_fd:
	if (trans.reg_fd >= 0)
		close(trans.reg_fd);
c2h_out:
	if (trans.c2h_fd >= 0)
		close(trans.c2h_fd);
h2c_out:
	if (trans.h2c_fd >= 0)
		close(trans.h2c_fd);
	app_running = false;
	return rc;

}

void *file_read (void *)
{
	char *read_allocated = NULL;
	int size;
	int iter, i, rc;
	struct stat st;
	unsigned long int file_size;
	int num_cpu;
#ifdef T_DEBUG
	struct timespec ts_start, ts_end;
	int k = 0;
#endif
	cpu_set_t cpuset;
	
	if (usecase_sel == 2 || usecase_sel == 7)
	size = in_height * in_width * BGR_MULTIPLIER;
	else
	size = in_height * in_width * YUV_MULTIPLIER;

	num_cpu = get_nprocs();
	if (num_cpu > 0 && num_cpu > 4) {
		CPU_ZERO(&cpuset);
		CPU_SET(3,&cpuset);
		rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
		if (rc != 0)
			printf("set affinity failed\n");
	}
	fstat(trans.infile_fd, &st);
	file_size = st.st_size;
	posix_memalign((void **)&read_allocated, 4096 /*alignment */ , size + 4096);
	if (!read_allocated) {
		fprintf(stderr, "OOM %u.\n", size + 4096);
		rc = -ENOMEM;
		return NULL;
	}

	iter = (file_size / size);
	for(i = 0; i< iter; i++)   {
#ifdef T_DEBUG
		if(k==1)
			clock_gettime(CLOCK_MONOTONIC, &ts_start);
#endif
		if (trans.infile_fd > 0) {
			rc = read_to_buffer(trans.infname, trans.infile_fd, read_allocated, size, 0);
			if (rc < 0) {
				printf("read to buffer failed size %d rc %d", size, rc);
				return NULL;
			}       
		}
		rc = cb_enque(&queue_file_frame, read_allocated);
		if (rc == 1) {
			do { 
			}while(queue_file_frame.index == queue_file_frame.capacity);
			cb_enque(&queue_file_frame, read_allocated);
		}
#ifdef T_DEBUG
		if(k==30){
			clock_gettime(CLOCK_MONOTONIC, &ts_end);
			double time_spent = (ts_end.tv_sec - ts_start.tv_sec) +
				(ts_end.tv_nsec - ts_start.tv_nsec) / BILLION;
			printf("###%s: Time %f\n", __func__, time_spent);
			k = 0;
		}
		k++;
#endif
	}

	return NULL;
}

void *pcie_dma_read(void *)
{
	int rc;
	int num_cpu;
	volatile unsigned int addr_high,addr_low, size, buffer_ready, read_complete;
	volatile unsigned int *transfer_done;
	volatile uint64_t addr;
	char *read_allocated = NULL;
	cpu_set_t cpuset;
#ifdef T_DEBUG
	struct timespec ts_start, ts_end;
	int k = 0;
#endif
	num_cpu = get_nprocs();
	if (num_cpu > 0 && num_cpu > 4) { 
		CPU_ZERO(&cpuset);
		CPU_SET(1,&cpuset);
		rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
		if (rc != 0)
			printf("set affinity failed\n");
	}
	if(usecase_sel == 2 || usecase_sel == 7)
		posix_memalign((void **)&read_allocated, 4096 /*alignment */ , (in_height * in_width * BGR_MULTIPLIER) + 4096);
	else 
		posix_memalign((void **)&read_allocated, 4096 /*alignment */ , (in_height * in_width * YUV_MULTIPLIER) + 4096);
	if (!read_allocated) {
		fprintf(stderr, "OOM %u.\n", size + 4096);
		rc = -ENOMEM;
		goto read_out;
	}

	trans.read_buffer = read_allocated;

	printf("Running read use case\n");

	while (1) {
		transfer_done = ((uint32_t *)(trans.map_base + PCIRC_READ_BUFFER_TRANSFER_DONE));
		buffer_ready = *((uint32_t *)(trans.map_base + PCIEP_READ_BUFFER_READY));
		read_complete = *((uint32_t *)(trans.map_base + PCIEP_READ_TRANSFER_COMPLETE));
#ifdef T_DEBUG
		if(k==1)
			clock_gettime(CLOCK_MONOTONIC, &ts_start);
#endif
		while (!(buffer_ready & 0x1))
		{
			buffer_ready = *((uint32_t *)(trans.map_base + PCIEP_READ_BUFFER_READY));
			read_complete = *((uint32_t *)(trans.map_base + PCIEP_READ_TRANSFER_COMPLETE));
			if (read_complete == 0xef)
				break;
		}

		if (read_complete == 0xef)
			break;

		addr_low = *((uint32_t *)(trans.map_base + PCIEP_READ_BUFFER_ADDR_LOW));
		addr_high = *((uint32_t *)(trans.map_base + PCIEP_READ_BUFFER_ADDR_HIGH));
		addr = (uint64_t) addr_high << 32 | addr_low;
		size = *((uint32_t *) (trans.map_base + PCIEP_READ_BUFFER_SIZE));
		rc = cb_deque(&queue_file_frame, trans.read_buffer);
		if (rc == 1) {
			do {
			} while(queue_file_frame.index == 0);
			cb_deque(&queue_file_frame, trans.read_buffer);
		}
		rc = write_from_buffer((char *)H2C_DEVICE, trans.h2c_fd, trans.read_buffer, size, addr);
		if (rc < 0) {
			printf("write from buffer failed size %d rc %d", size, rc);
			goto out;
		}
#ifdef T_DEBUG
		if(k==30){
			clock_gettime(CLOCK_MONOTONIC, &ts_end);
			double time_spent = (ts_end.tv_sec - ts_start.tv_sec) +
				(ts_end.tv_nsec - ts_start.tv_nsec) / BILLION;
			printf("###%s: Time %f\n", __func__, time_spent);
			k = 0;
		}
		k++;
#endif
		
		*transfer_done = 0x1;
		while ((buffer_ready & 0x1)) {
			buffer_ready = *((uint32_t *)(trans.map_base + PCIEP_READ_BUFFER_READY));
		}
	}

out:
	printf("\n** Read done\n");
read_out:
	if (read_allocated) {
		free(read_allocated);
		read_allocated = NULL;
	}
	return NULL;
}

#define BILLION  1000000000.0
void  *pcie_dma_write(void *)
{
	int rc;
	volatile unsigned int addr_low,addr_high, size,  write_buffer_ready, write_complete;
	volatile uint64_t addr;
	volatile unsigned int *transfer_done;
	volatile unsigned int *ucase_params;
	char *write_allocated = NULL;
	int num_cpu;
	cpu_set_t cpuset;
#ifdef T_DEBUG
	struct timespec ts_start, ts_end;
	int k = 0;
#endif
	num_cpu = get_nprocs();
	if (num_cpu > 0 && num_cpu > 4) { 
		CPU_ZERO(&cpuset);
		CPU_SET(2,&cpuset);
		rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
		if (rc != 0)
			printf("set affinity failed\n");
	}

	if(usecase_sel == 2 || usecase_sel == 7)
	posix_memalign((void **)&write_allocated, 4096 /*alignment */ , (in_height * in_width * BGR_MULTIPLIER) + 4096);
	else 
	posix_memalign((void **)&write_allocated, 4096 /*alignment */ , (in_height * in_width * YUV_MULTIPLIER) + 4096);
	if (!write_allocated) {
		fprintf(stderr, "OOM %u.\n", size + 4096);
		rc = -ENOMEM;
		goto out;
	}
	trans.write_buffer = write_allocated;

	printf("Running write use case\n");

	while (1) {
		transfer_done = ((uint32_t *)(trans.map_base + PCIRC_WRITE_BUFFER_TRANSFER_DONE));
		write_buffer_ready = *((uint32_t *)(trans.map_base + PCIEP_WRITE_BUFFER_READY));
		write_complete = *((uint32_t *)(trans.map_base + PCIEP_WRITE_TRANSFER_COMPLETE));
		*transfer_done = 0x0;
#ifdef T_DEBUG
		if(k==1)
			clock_gettime(CLOCK_MONOTONIC, &ts_start);
#endif

		while (!(write_buffer_ready & 0x1))
		{
			write_buffer_ready = *((uint32_t *)(trans.map_base + PCIEP_WRITE_BUFFER_READY));
			write_complete = *((uint32_t *)(trans.map_base + PCIEP_WRITE_TRANSFER_COMPLETE));
			if (write_complete == 0xef)	{
				break;
			}
		}
		if (write_complete == 0xef) {
			break;
		}

		addr_low = *((uint32_t *)(trans.map_base + PCIEP_WRITE_BUFFER_ADDR_LOW));
		addr_high = *((uint32_t *)(trans.map_base + PCIEP_WRITE_BUFFER_ADDR_HIGH));
		addr = (uint64_t) addr_high << 32 | addr_low;
		size = *((uint32_t *) (trans.map_base + PCIEP_WRITE_BUFFER_SIZE));
		if (trans.c2h_fd) {
			do {
			}while (queue_frame.index >= 70);

			rc = read_to_buffer((char *)C2H_DEVICE, trans.c2h_fd, trans.write_buffer, size, addr);
			if (rc < 0) {
				printf("write function read to buffer failed size %d rc %d\n", size, rc);
				goto out;
			}

			cb_enque(&queue_frame, trans.write_buffer);
		}
#ifdef T_DEBUG
		if(k==30){
			clock_gettime(CLOCK_MONOTONIC, &ts_end);
			double time_spent = (ts_end.tv_sec - ts_start.tv_sec) +
				(ts_end.tv_nsec - ts_start.tv_nsec) / BILLION;
			printf("###%s: Time %f\n", __func__, time_spent);
			k = 0;
		}
		k++;
#endif

		*transfer_done = 0x1;
		while(write_buffer_ready) {
			write_buffer_ready = *((uint32_t *)(trans.map_base + PCIEP_WRITE_BUFFER_READY));
		}
	}
out:
	printf("** Write done\n");
	if (write_allocated) {
		free(write_allocated);
		write_allocated = NULL;
	}
	/* Setting Use case to zero */
	u_case = 0;
	ucase_params = ((uint32_t *)(trans.map_base + PCIRC_UCASE_SET));
	*ucase_params = u_case & 0xFFFFFFFF;
	return NULL;
}
