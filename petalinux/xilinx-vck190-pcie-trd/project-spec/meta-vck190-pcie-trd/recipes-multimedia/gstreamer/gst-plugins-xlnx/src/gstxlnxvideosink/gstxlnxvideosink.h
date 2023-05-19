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

#ifndef _GST_XLNX_VIDEO_SINK_H_
#define _GST_XLNX_VIDEO_SINK_H_

#include <gst/gst.h>
#include <gst/video/video.h>

G_BEGIN_DECLS

GType gst_xlnx_video_sink_get_type(void);
#define GST_TYPE_XLNX_VIDEO_SINK \
    (gst_xlnx_video_sink_get_type())
#define GST_XLNX_VIDEO_SINK(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_XLNX_VIDEO_SINK,GstXlnxVideoSink))
#define GST_XLNX_VIDEO_SINK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_XLNX_VIDEO_SINK,GstXlnxVideoSinkClass))
#define GST_IS_XLNX_VIDEO_SINK(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_XLNX_VIDEO_SINK))
#define GST_IS_XLNX_VIDEO_SINK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_XLNX_VIDEO_SINK))
#define GST_XLNX_VIDEO_SINK_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_XLNX_VIDEO_SINK,GstXlnxVideoSinkClass))

typedef struct _GstXlnxVideoSink GstXlnxVideoSink;
typedef struct _GstXlnxVideoSinkClass GstXlnxVideoSinkClass;

typedef enum {
  GST_XLNX_SINK_NONE = -1,
  GST_XLNX_SINK_DP = 0,
  GST_XLNX_SINK_HDMI = 1,
} GstXlnxVideoSinkType;

struct _GstXlnxVideoSink
{
  GstBin bin;

  GstElement *video_sink;
  GstPad *ghost_pad;

  gboolean on_error;
  GstXlnxVideoSinkType sink_type;
  gboolean is_fullscreen_overlay;
  gboolean is_autoselect_driver;
};

struct _GstXlnxVideoSinkClass
{
  GstBinClass parent_class;
};

G_END_DECLS

#endif
