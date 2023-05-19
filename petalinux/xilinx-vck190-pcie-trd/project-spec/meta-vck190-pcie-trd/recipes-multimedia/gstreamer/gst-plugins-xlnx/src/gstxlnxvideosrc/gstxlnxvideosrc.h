/*
 * Copyright (C) 2017 – 2018 Xilinx, Inc.  All rights reserved.
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
#ifndef _GST_XLNX_VIDEO_SRC_H_
#define _GST_XLNX_VIDEO_SRC_H_

#include <gst/gst.h>
#include <gst/video/video.h>
#include "video.h"
G_BEGIN_DECLS

GType gst_xlnx_video_src_get_type(void);
#define GST_TYPE_XLNX_VIDEO_SRC \
    (gst_xlnx_video_src_get_type())
#define GST_XLNX_VIDEO_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_XLNX_VIDEO_SRC,GstXlnxVideoSrc))
#define GST_XLNX_VIDEO_SRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_XLNX_VIDEO_SRC,GstXlnxVideoSrcClass))
#define GST_IS_XLNX_VIDEO_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_XLNX_VIDEO_SRC))
#define GST_IS_XLNX_VIDEO_SRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_XLNX_VIDEO_SRC))
#define GST_XLNX_VIDEO_SRC_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_XLNX_VIDEO_SRC,GstXlnxVideoSrcClass))

typedef struct _GstXlnxVideoSrc GstXlnxVideoSrc;
typedef struct _GstXlnxVideoSrcClass GstXlnxVideoSrcClass;

typedef enum {
  GST_XLNX_SRC_NONE = -1,
  GST_XLNX_SRC_VIVID = 0,
  GST_XLNX_SRC_MIPI,
  GST_XLNX_SRC_HDMI,
  GST_XLNX_SRC_USBCAM,
  GST_XLNX_SRC_TPG,
  GST_XLNX_SRC_MIPI_QUAD_VC0,
  GST_XLNX_SRC_MIPI_QUAD_VC1,
  GST_XLNX_SRC_MIPI_QUAD_VC2,
  GST_XLNX_SRC_MIPI_QUAD_VC3
} GstXlnxVideoSrcType;

struct _GstXlnxVideoSrc
{
  GstBin bin;

  GstElement *video_src;
  GstPad *ghost_pad;
  gboolean on_error;
  GstXlnxVideoSrcType src_type;
  gint src_idx;
  gboolean auto_select;
  struct vlib_config_data *cfg;
};

struct _GstXlnxVideoSrcClass
{
  GstBinClass parent_class;
};

G_END_DECLS

#endif
