/*********************************************************************
 * Copyright (C) 2021 Xilinx, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 ********************************************************************/

#ifndef _PCIE_MAIN_H_
#define _PCIE_MAIN_H_

#include <stdio.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <string.h>
#include <gst/app/gstappsink.h>
#include <gst/allocators/gstdmabuf.h>
#include <pcie_abstract.h>
#include <gst/video/video.h>
#include <glib.h>

#define YUY2_MULTIPLIER                 2
#define BGR_MULTIPLIER			3
#define INPUT_SRC                       "/dev/video0"
#define VGST_V4L2_IO_MODE_DMABUF_EXPORT 4
#define MAX_FRAME_RATE_DENOM            1
#define VIDEOPARSE_FORMAT_YUY2          "YUY2"
#define PCIE_GST_APP_FAIL               -1
#define VGST_FILTER_KERNEL_NAME         "filter2d_pl_accel"
#define HOST_APP_REG_READ_TIMEOUT       1 /* in seconds */
#define RW_DONE_SET_AND_CLEAR_DELAY     1 /* in seconds */

typedef struct {
    guint64 length;
    guint input_format, fps, kernel_name;
    guint filter_preset, kernel_mode, usecase;
    resolution input_res;
} host_params;

typedef struct {
    gint fd;
    guint sourceid, dma_map_idx;
    GMainLoop *loop;
    gboolean eos_flag, exit_thread;
    host_params h_param;
    dma_buf_imp dma_import;
    dma_buf_export dma_export, dma_map[MAX_BUFFER_POOL_SIZE];
    GstElement *inputsrc, *vvas_xfilter, *perf;
    GstElement *pipeline, *pciesrc, *capsfilter, *pciesink, *hdmisink;
    GstElement *dpubin;
    guint64 appsrc_framecnt, appsink_framecnt;
    guint64 read_offset, yuv_frame_size, export_fd_size;
} App;

typedef enum {
    VGST_FORMAT_YUY2 = 1,
    VGST_FORMAT_BGR = 2,
    VGST_FORMAT_MAX,
} VGST_FORMAT_TYPE;

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

typedef enum {
    VGST_FILTER_MODE_SW = 0,
    VGST_FILTER_MODE_HW,
    VGST_FILTER_MODE_MAX,
} VGST_FILTER_MODE;

typedef enum {
    VGST_FILTER_PRESET_BLUR = 0,
    VGST_FILTER_PRESET_EDGE,
    VGST_FILTER_PRESET_HORIZONTAL_EDGE,
    VGST_FILTER_PRESET_VERTICAL_EDGE,
    VGST_FILTER_PRESET_EMBOSS,
    VGST_FILTER_PRESET_HORIZONTAL_GRADIENT,
    VGST_FILTER_PRESET_VERTICAL_GRADIENT,
    VGST_FILTER_PRESET_IDENTITY,
    VGST_FILTER_PRESET_SHARPEN,
    VGST_FILTER_PRESET_HORIZONTAL_SOBEL,
    VGST_FILTER_PRESET_VERTICAL_SOBEL,
    VGST_FILTER_PRESET_MAX,
} VGST_FILTER_PRESET;

static const char * const filter_presets[] = {
	[VGST_FILTER_PRESET_BLUR] = "blur",
	[VGST_FILTER_PRESET_EDGE] = "edge",
	[VGST_FILTER_PRESET_HORIZONTAL_EDGE] = "horizontal edge",
	[VGST_FILTER_PRESET_VERTICAL_EDGE] = "vertical edge",
	[VGST_FILTER_PRESET_EMBOSS] = "emboss",
	[VGST_FILTER_PRESET_HORIZONTAL_GRADIENT] = "horizontal gradient",
	[VGST_FILTER_PRESET_VERTICAL_GRADIENT] = "vertical gradient",
	[VGST_FILTER_PRESET_IDENTITY] = "identity",
	[VGST_FILTER_PRESET_SHARPEN] = "sharpen",
	[VGST_FILTER_PRESET_HORIZONTAL_SOBEL] = "horizontal sobel",
	[VGST_FILTER_PRESET_VERTICAL_SOBEL] = "vertical sobel",
	[VGST_FILTER_PRESET_MAX] = NULL

};
#endif /* _PCIE_MAIN_H_ */

