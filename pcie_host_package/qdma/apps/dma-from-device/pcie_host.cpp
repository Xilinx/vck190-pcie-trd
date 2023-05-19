/*
 * This file is part of the Xilinx DMA IP Core driver tools for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include "pcie_host.h"
#include "main.h"
#define _BSD_SOURCE
#define _XOPEN_SOURCE 500
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
#define MAP_SIZE (32*1024UL)
#define MAP_MASK (MAP_SIZE - 1)

#define PCIRC_GET_FILE_LENGTH      0x04
#define PCIRC_READ_BUFFER_TRANSFER_DONE 0x2c
#define PCIRC_WRITE_BUFFER_TRANSFER_DONE 0x30
#define PCIRC_HOST_DONE			  0x34
#define PCIRC_FILTER_PARAMS        0x14
#define PCIRC_RAW_RESOLUTION      0x18

#define PCIEP_READ_BUFFER_READY   0x3c
#define PCIEP_READ_BUFFER_ADDR   0x40
#define PCIEP_READ_BUFFER_OFFSET 0x44
#define PCIEP_READ_BUFFER_SIZE   0x48
#define PCIEP_WRITE_BUFFER_READY   0x4c
#define PCIEP_WRITE_BUFFER_ADDR   0x50
#define PCIEP_WRITE_BUFFER_OFFSET 0x54
#define PCIEP_WRITE_BUFFER_SIZE   0x58
#define PCIEP_READ_TRANSFER_COMPLETE   0x5c
#define PCIEP_WRITE_TRANSFER_COMPLETE  0x60

//#define H2C_DEVICE "/dev/xdma0_h2c_0"
//#define C2H_DEVICE "/dev/xdma0_c2h_0"
#define H2C_DEVICE "/dev/qdma02000-MM-0"
#define C2H_DEVICE "/dev/qdma02000-MM-1"
#define REG_DEVICE_NAME "/sys/bus/pci/devices/0000:02:00.0/resource0"
#define COUNT_DEFAULT 		  (1)
#define SIZE_DEFAULT		  (32)

/* Frame rate options */
#define FPS_MIN               (30)
#define FPS_MAX               (60)
#define FPS_DEFAULT           (FPS_MIN)

#define FILTER_TYPE_DEFAULT         (1)

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

struct pcie_transfer {
	int infile_fd;
	int h2c_fd;
	int c2h_fd;
	int reg_fd;
	void *map_base;
	char *read_buffer;
	char *write_buffer;
	char *infname;
} trans;

int filter_type_val = FILTER_TYPE_DEFAULT;
unsigned int in_width = INPUT_WIDTH_DEFAULT;
unsigned int in_height = INPUT_HEIGHT_DEFAULT;
unsigned int fps = FPS_DEFAULT;

circular_buffer queue_frame;
circular_buffer queue_file_frame;
bool app_running = true;

static struct option const long_opts[] = {
	{"input file name (Raw file yuy2 or yuv)", required_argument, NULL, 'i'},
	{"input resolution. Range of this value is [1920x1080 or 3840x2160]: 1920x1080 represents width x height of input video.\n \
	Default:1920x1080", required_argument, NULL, 'd'},
	{"input Format. Range of this value is [0 to 10]: this parameter is used for setting type of filter.\n \
        Default:0", required_argument, NULL, 't'},
	{"help", no_argument, NULL, 'h'},
	{0, 0, 0, 0}
};

static void usage(const char *name)
{
	int i = 0;

	fprintf(stdout, "%s\n\n", name);
	fprintf(stdout, "usage: %s [OPTIONS]\n\n", name);

	for (i=0 ; i<(sizeof(long_opts)/sizeof(long_opts[0])) - 2; i++)
		fprintf(stdout, "  -%c represents %s.\n",
			long_opts[i].val, long_opts[i].name);
	fprintf(stdout, "  -%c (%s) print usage help and exit\n",
		long_opts[i].val, long_opts[i].name);
	i++;
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

int cmaincall(int argc, char *argv[])
{
	int cmd_opt;
	char *h2c_device = H2C_DEVICE;
	char *c2h_device = C2H_DEVICE;
	char *infname;
	char cross_sign = 0;

	while ((cmd_opt =
		getopt_long(argc, argv, "vhc:i:d:t:", long_opts,
			    NULL)) != -1) {
		switch (cmd_opt) {
		case 0:
			/* long option */
			break;

		case 'i':
			infname = strdup(optarg);
			break;


		case 'd':
			/* Set input resolution */
			sscanf(optarg, "%u%c%u", &in_width, &cross_sign, &in_height);

			if (!((in_width == 1920 && in_height == 1080) || (in_width == 3840 && in_height == 2160))) {
				in_width = 1920;
				in_height = 1080;
				printf("-d Dimensions should be 1920x1080 or 3840x2160, setting to default 1920x1080\n");
			}
			break;

		case 't':
                        /* Set filter type */
                        filter_type_val = getopt_integer(optarg);
                        if (filter_type_val < FILTER_BLUR && filter_type_val > FILTER_VSOBEL) {
                                filter_type_val = FILTER_TYPE_DEFAULT;
                                printf("Input value should be between 0 to 10, setting to default %d \n", filter_type_val);
                        }
                        break;

		/* print usage help and exit */
		case 'h':
	default:
			usage(argv[0]);
			exit(0);
			break;
		}
	}

	return test_dma(h2c_device, c2h_device, infname);
}

static int test_dma(char *h2c_device, char *c2h_device, char *infname)
{
	ssize_t rc;
	volatile unsigned int *host_done;
	volatile unsigned int *infile_len;
	unsigned long int file_len;
	filter_type fil_type;
	volatile unsigned int *input_res = NULL;
	volatile unsigned int *filter_params;

	pthread_t thread1, thread2, thread3;

	trans.h2c_fd = open(h2c_device, O_RDWR);
	if (trans.h2c_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
			h2c_device, trans.h2c_fd);
		perror("open device");
                app_running = false;
		return -EINVAL;
	}

	trans.c2h_fd = open(c2h_device, O_RDWR);
	if (trans.c2h_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
			c2h_device, trans.c2h_fd);
		perror("open device");
		goto h2c_out;
	}


	trans.reg_fd = open(REG_DEVICE_NAME, O_RDWR);
	if (trans.reg_fd < 0) {
        
	fprintf(stderr, "unable to open device %s, %d.\n",
            	REG_DEVICE_NAME, trans.reg_fd);
        	perror("open reg device failed");
		goto c2h_out;
         }

	trans.infname = infname;
	trans.infile_fd = open(infname, O_RDONLY);
	if (trans.infile_fd < 0) {
		fprintf(stderr, "unable to open input file %s, %d.\n",
			infname, trans.infile_fd);
		printf("open input file failed");
		rc = -EINVAL;
		goto reg_out;
	}

	/* map one page */
	trans.map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, trans.reg_fd, 0);
	if (trans.map_base == (void *)-1) {
		printf("error unmap\n");
		printf("Memory mapped at address %p.\n", trans.map_base);
		fflush(stdout);
		goto reg_out;
	
	}

	/* Get the input file length  */
	if (trans.infile_fd > 0) {
		file_len = lseek(trans.infile_fd, 0, SEEK_END);
		if (file_len == (off_t)-1)
		{
			printf("failed to lseek %s numbytes %lu\n", trans.infname, file_len);
			exit(EXIT_FAILURE);
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
	
	rc = cb_init(&queue_frame, 240, in_width * in_height * 2);
	if (rc < 0)
		goto reg_out;

	rc = cb_init(&queue_file_frame, 120, in_width * in_height * 2);
        if (rc < 0)
                goto free_qframe;

//        pthread_create( &thread2, NULL, &file_read, NULL);
//	pthread_create( &thread1, NULL, &pcie_dma_read, NULL);
	pthread_create( &thread3, NULL, &pcie_dma_write, NULL);

//	pthread_join( thread3, NULL);
//	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);
	
	while(queue_frame.index);

	/*set host done interrupt */
	host_done = ((uint32_t *)(trans.map_base + PCIRC_HOST_DONE));
	*host_done = 0x1;

	/* clear file length and encoder parameters */
	*infile_len = 0x0;
	free(queue_file_frame.buffer);
free_qframe:
	free(queue_frame.buffer);
reg_out:
	if (munmap(trans.map_base, MAP_SIZE) == -1)
		printf("error unmap\n");
	close(trans.reg_fd);

infile_out:
	if (trans.infile_fd >= 0)
		close(trans.infile_fd);

c2h_out:
	close(trans.c2h_fd);

h2c_out:
	close(trans.h2c_fd);
        app_running = false;
	return rc;
}


void *file_read (void *vargp)
{
        char *read_allocated = NULL;
        int size = in_height * in_width * 2;
        int iter, i, rc;
        struct stat st;
        unsigned long int file_size;
        int num_cpu;

        cpu_set_t cpuset;

        num_cpu = get_nprocs();
        if (num_cpu > 0 && num_cpu < 4) {
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
        }

	return NULL;
}

void *pcie_dma_read(void *vargp)
{
	int rc, i;
	int num_cpu;
	volatile unsigned int addr, size, buffer_ready, read_complete;
	volatile unsigned long int offset = 0;
	volatile unsigned int lsb_offset, msb_offset;
	volatile unsigned int *transfer_done;
	char *read_allocated = NULL;
	cpu_set_t cpuset;
	
	num_cpu = get_nprocs();
	if (num_cpu > 0 && num_cpu < 4) { 
		CPU_ZERO(&cpuset);
		CPU_SET(1,&cpuset);
		rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
		if (rc != 0)
			printf("set affinity failed\n");
	}
	
	posix_memalign((void **)&read_allocated, 4096 /*alignment */ , (in_height * in_width *2) + 4096);
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

		while (!(buffer_ready & 0x1))
		{
			buffer_ready = *((uint32_t *)(trans.map_base + PCIEP_READ_BUFFER_READY));
			read_complete = *((uint32_t *)(trans.map_base + PCIEP_READ_TRANSFER_COMPLETE));
			if (read_complete == 0xef)
				break;
		}

		if (read_complete == 0xef)
			break;

		addr = *((uint32_t *)(trans.map_base + PCIEP_READ_BUFFER_ADDR));
		size = *((uint32_t *) (trans.map_base + PCIEP_READ_BUFFER_SIZE));
		lsb_offset = *((uint32_t *) (trans.map_base + PCIEP_READ_BUFFER_OFFSET));
		msb_offset = *((uint32_t *) (trans.map_base + PCIEP_READ_BUFFER_READY));	
		offset = lsb_offset | ((unsigned long int)(msb_offset & 0xFFFF0000) << 16);

		rc = cb_deque(&queue_file_frame, trans.read_buffer);
                if (rc == 1) {
                        do {
                        } while(queue_file_frame.index == 0);
                        cb_deque(&queue_file_frame, trans.read_buffer);
                }

		rc = write_from_buffer(H2C_DEVICE, trans.h2c_fd, trans.read_buffer, size, addr);
		if (rc < 0) {
			printf("write from buffer failed size %d rc %d", size, rc);
			goto out;
		}

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
void  *pcie_dma_write(void * argp)
{
	int rc;
	volatile unsigned int addr, size, offset, write_buffer_ready, write_complete;
	volatile unsigned int *transfer_done;
	char *write_allocated = NULL;
	int num_cpu;
	cpu_set_t cpuset;

	num_cpu = get_nprocs();
	if (num_cpu > 0 && num_cpu < 4) { 
		CPU_ZERO(&cpuset);
		CPU_SET(2,&cpuset);
		rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
		if (rc != 0)
			printf("set affinity failed\n");
	}

	posix_memalign((void **)&write_allocated, 4096 /*alignment */ , (in_height * in_width *2) + 4096);
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

		addr = *((uint32_t *)(trans.map_base + PCIEP_WRITE_BUFFER_ADDR));
		size = *((uint32_t *) (trans.map_base + PCIEP_WRITE_BUFFER_SIZE));
		offset = *((uint32_t *) (trans.map_base + PCIEP_WRITE_BUFFER_OFFSET));

		if (trans.c2h_fd) {
			do {
			}while (queue_frame.index >= 70);
			
			rc = read_to_buffer(C2H_DEVICE, trans.c2h_fd, trans.write_buffer, size, addr);
			if (rc < 0) {
				printf("write function read to buffer failed size %d rc %d\n", size, rc);
				goto out;
			}

			cb_enque(&queue_frame, trans.write_buffer);
		}

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

	return NULL;
}
