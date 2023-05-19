/*
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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

#ifndef PCIE_HOST_APP_CIRC_H
#define PCIE_HOST_APP_CIRC_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct circular_buffer
{
char *buffer;
char *buffer_end;
size_t capacity;
size_t index;
size_t sz;
char *head;
char *tail;
} circular_buffer;

extern "C"{
extern circular_buffer queue_frame;
extern bool app_running;
/**
 * mipi_displayonhost 	: Function to provide control from host to Endpoint and initialize a mipi capture pipeline from endpoint (i.e., appsrc)    *			    and process frames into filter plugin then to displayonhost through appsink. 
 * frm 			: A Qtwindow frame to display on host.
 * c2h_device		: qdma channel to host device node for dma read transaction.
 */
int mipi_displayonhost(struct MainWindow *frm,const char *c2h_device );

/**
 * host2host   : Function to provide control from host to Endpoint and transfer a video file from host to EP via pcie. (i.e., appsrc)    
 *               and process frames into filter plugin then to displayonhost through appsink. 
 * frm         : A Qtwindow frame to display on host.
 * h2c_device  : qdma host to channel device node for dma write transaction
 * c2h_device  : qdma channel to host device node for dma read transaction.
 */
int host2host(struct MainWindow *frm,const char *h2c_device,const char *c2h_device);
int host2kmssink_with_filter(const char *h2c_device);

/**
 * host2host_without_filter   : Function to provide control from host to Endpoint and transfer a video file from host to EP via pcie.(i.e.,appsrc)
 *               		and process frames into filter plugin then to displayonhost through appsink. 
 * frm         		      : A Qtwindow frame to display on host.
 * h2c_device  		      : qdma host to channel device node for dma write transaction
 * c2h_device  : qdma channel to host device node for dma read transaction.
 */

int host2host_without_filter(struct MainWindow *frm,const char *h2c_device,const char *c2h_device);

/**
 * pcie_dma_read :  Thread function to start dma write transaction from host to channel.
 * @vargp : takes nothing as argument.
 */
void *pcie_dma_read(void *);
/**
 * pcie_dma_write : Thread function to start dma read transaction from channel to host.
 * @vargp : takes nothing as argument.
 */
void *pcie_dma_write(void *);
/**
 * file_read : Read input file and strat dma transaction between channel and host.
 */
void *file_read(void * vargp);
int cb_deque(circular_buffer *cb, char *data);
int cmaincall(struct MainWindow *frm);


}
#endif // PCIE_HOST_APP_H
