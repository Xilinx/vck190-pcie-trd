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

/**
 * SECTION:xlnxvideosrc
 *
 * This source element is a BIN consists of v4l2src as child
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v xlnxvideosrc src-type=mipi ! "video/x-raw,width=640,height=480,format=YUY2" ! fakesink -v
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstxlnxvideosrc.h"

GST_DEBUG_CATEGORY (gst_xlnx_video_src_debug);
#define GST_CAT_DEFAULT gst_xlnx_video_src_debug

#define parent_class gst_xlnx_video_src_parent_class

G_DEFINE_TYPE (GstXlnxVideoSrc, gst_xlnx_video_src, GST_TYPE_BIN);

static void gst_xlnx_video_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * param_spec);
static void gst_xlnx_video_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * param_spec);
static GstStateChangeReturn gst_xlnx_video_src_change_state (GstElement *
    element, GstStateChange transition);
static void gst_xlnx_video_src_dispose (GObject * object);
static void xlnx_video_src_update_source (GstXlnxVideoSrc * self,
    GstElement * video_src);

static GstStaticPadTemplate gst_xlnx_video_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

#define GST_TYPE_XLNX_VIDEO_SRC_TYPE (xlnx_video_src_get_type ())
GType xlnx_video_src_get_type (void);

enum
{
  PROP_0,
  PROP_SRC_TYPE,
};

GType
xlnx_video_src_get_type (void)
{
  static GType xlnx_source_type = 0;

  if (!xlnx_source_type) {
    static const GEnumValue src_types[] = {
      {GST_XLNX_SRC_NONE, "Video Source NONE", "none"},
      {GST_XLNX_SRC_VIVID, "Virtual Video Device", "vivid"},
      {GST_XLNX_SRC_MIPI, "MIPI CSI2 Rx", "mipi"},
      {GST_XLNX_SRC_HDMI, "HDMI Input", "hdmi"},
      {GST_XLNX_SRC_USBCAM, "USB Webcam", "usbcam"},
      {GST_XLNX_SRC_TPG, "Test Pattern Generator", "tpg"},
      {GST_XLNX_SRC_MIPI_QUAD_VC0, "MIPI Quad Virtual Channel 0",
          "mipi_quad_vc0"},
      {GST_XLNX_SRC_MIPI_QUAD_VC1, "MIPI Quad Virtual Channel 1",
          "mipi_quad_vc1"},
      {GST_XLNX_SRC_MIPI_QUAD_VC2, "MIPI Quad Virtual Channel 2",
          "mipi_quad_vc2"},
      {GST_XLNX_SRC_MIPI_QUAD_VC3, "MIPI Quad Virtual Channel 3",
          "mipi_quad_vc3"},
      {0, NULL, NULL}
    };
    xlnx_source_type =
        g_enum_register_static ("GstXlnxVideoSrcType", src_types);
  }
  return xlnx_source_type;
}

#define DEFAULT_VIDEO_SRC_TYPE GST_XLNX_SRC_NONE

static const char *
get_nickname_from_srctype (GstXlnxVideoSrcType srctype)
{
  switch (srctype) {
    case GST_XLNX_SRC_VIVID:
      return "virtual";
    case GST_XLNX_SRC_MIPI:
      return "mipi";
    case GST_XLNX_SRC_HDMI:
      return "hdmi";
    case GST_XLNX_SRC_USBCAM:
      return "usb";
    case GST_XLNX_SRC_TPG:
      return "test";
    case GST_XLNX_SRC_MIPI_QUAD_VC0:
    case GST_XLNX_SRC_MIPI_QUAD_VC1:
    case GST_XLNX_SRC_MIPI_QUAD_VC2:
    case GST_XLNX_SRC_MIPI_QUAD_VC3:
      return "multicam";
    default:
      return NULL;
  }
}

/* get V4L2 fourcc from GST fourcc */
static guint32
get_v4l2_fourcc (GstVideoInfo * info)
{
  guint32 fourcc = 0;

  // FIXME: not using fourcc_nc (non-contiguous) now 
  switch (GST_VIDEO_INFO_FORMAT (info)) {
    case GST_VIDEO_FORMAT_I420:
      fourcc = V4L2_PIX_FMT_YUV420;

      break;
    case GST_VIDEO_FORMAT_YUY2:
      fourcc = V4L2_PIX_FMT_YUYV;
      break;
    case GST_VIDEO_FORMAT_UYVY:
      fourcc = V4L2_PIX_FMT_UYVY;
      break;
    case GST_VIDEO_FORMAT_YV12:
      fourcc = V4L2_PIX_FMT_YVU420;
      break;
    case GST_VIDEO_FORMAT_Y41B:
      fourcc = V4L2_PIX_FMT_YUV411P;
      break;
    case GST_VIDEO_FORMAT_Y42B:
      fourcc = V4L2_PIX_FMT_YUV422P;
      break;
    case GST_VIDEO_FORMAT_NV12:
      fourcc = V4L2_PIX_FMT_NV12;
      break;
    case GST_VIDEO_FORMAT_NV21:
      fourcc = V4L2_PIX_FMT_NV21;
      break;
    case GST_VIDEO_FORMAT_NV16:
      fourcc = V4L2_PIX_FMT_NV16;
      break;
    case GST_VIDEO_FORMAT_NV61:
      fourcc = V4L2_PIX_FMT_NV61;
      break;
    case GST_VIDEO_FORMAT_NV24:
      fourcc = V4L2_PIX_FMT_NV24;
      break;
    case GST_VIDEO_FORMAT_YVYU:
      fourcc = V4L2_PIX_FMT_YVYU;
      break;
    case GST_VIDEO_FORMAT_RGB15:
      fourcc = V4L2_PIX_FMT_RGB555;
      break;
    case GST_VIDEO_FORMAT_RGB16:
      fourcc = V4L2_PIX_FMT_RGB565;
      break;
    case GST_VIDEO_FORMAT_RGB:
      fourcc = V4L2_PIX_FMT_RGB24;
      break;
    case GST_VIDEO_FORMAT_BGR:
      fourcc = V4L2_PIX_FMT_BGR24;
      break;
    case GST_VIDEO_FORMAT_xRGB:
      fourcc = V4L2_PIX_FMT_RGB32;
      break;
    case GST_VIDEO_FORMAT_ARGB:
      fourcc = V4L2_PIX_FMT_RGB32;
      break;
    case GST_VIDEO_FORMAT_BGRx:
      fourcc = V4L2_PIX_FMT_BGR32;
      break;
    case GST_VIDEO_FORMAT_BGRA:
      fourcc = V4L2_PIX_FMT_BGR32;
      break;
    case GST_VIDEO_FORMAT_GRAY8:
      fourcc = V4L2_PIX_FMT_GREY;
      break;
    case GST_VIDEO_FORMAT_GRAY16_LE:
      fourcc = V4L2_PIX_FMT_Y16;
      break;
    case GST_VIDEO_FORMAT_GRAY16_BE:
      fourcc = V4L2_PIX_FMT_Y16_BE;
      break;
    default:
      GST_WARNING ("unsupported gst fourcc");
      break;
  }
  return fourcc;
}

static guint
xlnx_video_src_install_child_properties (GstXlnxVideoSrc * self,
    GstElement * video_src)
{
  guint num_props = 0;
  GParamSpec **props = NULL;
  GParamSpec **self_props = NULL;
  guint self_num_props = 0;
  int i;

  props =
      g_object_class_list_properties (G_OBJECT_CLASS (GST_ELEMENT_GET_CLASS
          (video_src)), &num_props);
  self_props =
      g_object_class_list_properties (G_OBJECT_CLASS (GST_ELEMENT_GET_CLASS
          (self)), &self_num_props);

  GST_LOG_OBJECT (self, "number of properties of v4l2src = %d and self = %d",
      num_props, self_num_props);

  for (i = 0; i < num_props; i++) {
    GType param_type;
    GParamSpec *pspec = props[i];
    GParamSpec *our_pspec;

    GST_LOG_OBJECT (self,
        "Child Property : name = %s, nickname = %s and blurb = %s",
        g_param_spec_get_name (pspec), g_param_spec_get_nick (pspec),
        g_param_spec_get_blurb (pspec));

    if (g_object_class_find_property (G_OBJECT_CLASS (GST_ELEMENT_GET_CLASS
                (self)), g_param_spec_get_name (pspec))) {
      continue;
    }

    param_type = G_PARAM_SPEC_TYPE (pspec);

    if (param_type == G_TYPE_PARAM_BOOLEAN) {   /* Boolean */
      GParamSpecBoolean *src_pspec = G_PARAM_SPEC_BOOLEAN (pspec);

      our_pspec = g_param_spec_boolean (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->default_value, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_CHAR) {       /* Char */
      GParamSpecChar *src_pspec = G_PARAM_SPEC_CHAR (pspec);

      our_pspec = g_param_spec_char (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_UCHAR) {      /* Unsigned Char */
      GParamSpecUChar *src_pspec = G_PARAM_SPEC_UCHAR (pspec);

      our_pspec = g_param_spec_char (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_INT) {        /* Integer */
      GParamSpecInt *src_pspec = G_PARAM_SPEC_INT (pspec);

      our_pspec = g_param_spec_int (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_UINT) {       /* Unsigned Integer */
      GParamSpecUInt *src_pspec = G_PARAM_SPEC_UINT (pspec);

      our_pspec = g_param_spec_uint (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_LONG) {       /* Long */
      GParamSpecLong *src_pspec = G_PARAM_SPEC_LONG (pspec);

      our_pspec = g_param_spec_long (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_ULONG) {      /* Unsigned Long */
      GParamSpecULong *src_pspec = G_PARAM_SPEC_ULONG (pspec);

      our_pspec = g_param_spec_ulong (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_INT64) {      /* Integer64 */
      GParamSpecInt64 *src_pspec = G_PARAM_SPEC_INT64 (pspec);

      our_pspec = g_param_spec_int64 (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_UINT64) {     /* Unsigned Integer64 */
      GParamSpecUInt64 *src_pspec = G_PARAM_SPEC_UINT64 (pspec);

      our_pspec = g_param_spec_uint64 (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_FLOAT) {      /* Float */
      GParamSpecFloat *src_pspec = G_PARAM_SPEC_FLOAT (pspec);

      our_pspec = g_param_spec_float (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_FLOAT) {      /* Double */
      GParamSpecDouble *src_pspec = G_PARAM_SPEC_DOUBLE (pspec);

      our_pspec = g_param_spec_double (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->minimum, src_pspec->maximum, src_pspec->default_value,
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_ENUM) {       /* Enum */
      GParamSpecEnum *src_pspec = G_PARAM_SPEC_ENUM (pspec);

      our_pspec = g_param_spec_enum (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          pspec->value_type, src_pspec->default_value, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_FLAGS) {      /* Flags */
      GParamSpecFlags *src_pspec = G_PARAM_SPEC_FLAGS (pspec);

      our_pspec = g_param_spec_flags (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          pspec->value_type, src_pspec->default_value, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_STRING) {     /* String */
      GParamSpecString *src_pspec = G_PARAM_SPEC_STRING (pspec);

      our_pspec = g_param_spec_string (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->default_value, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_PARAM) {      /* Param */
      our_pspec = g_param_spec_param (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          pspec->value_type, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_BOXED) {      /*Boxed */
      our_pspec = g_param_spec_boxed (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          pspec->value_type, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_POINTER) {    /*Pointer */
      our_pspec = g_param_spec_pointer (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          pspec->flags);
    } else if (param_type == G_TYPE_PARAM_OBJECT) {     /* Object */
      our_pspec = g_param_spec_object (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          pspec->value_type, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_UNICHAR) {    /* UniChar */
      GParamSpecUnichar *src_pspec = G_PARAM_SPEC_UNICHAR (pspec);

      our_pspec = g_param_spec_unichar (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->default_value, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_VALUE_ARRAY) {        /* ValurArray */
      GParamSpecValueArray *src_pspec = G_PARAM_SPEC_VALUE_ARRAY (pspec);

      our_pspec = g_param_spec_value_array (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->element_spec, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_GTYPE) {      /* GType */
      GParamSpecGType *src_pspec = G_PARAM_SPEC_GTYPE (pspec);

      our_pspec = g_param_spec_gtype (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->is_a_type, pspec->flags);
    } else if (param_type == G_TYPE_PARAM_VARIANT) {    /* Variant */
      GParamSpecVariant *src_pspec = G_PARAM_SPEC_VARIANT (pspec);

      our_pspec = g_param_spec_variant (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->type, src_pspec->default_value, pspec->flags);
    } else {
      GST_WARNING_OBJECT ("Unsupported property type %s for property %s",
          g_type_name (param_type), g_param_spec_get_name (pspec));
      G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (video_src), i, pspec);
      continue;
    }
    g_object_class_install_property (G_OBJECT_CLASS (GST_ELEMENT_GET_CLASS
            (self)), self_num_props, our_pspec);
    self_num_props++;
  }
  g_free (props);
  g_free (self_props);
  return self_num_props;
}


static void
gst_xlnx_video_src_class_init (GstXlnxVideoSrcClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_xlnx_video_src_set_property;
  gobject_class->get_property = gst_xlnx_video_src_get_property;
  gobject_class->dispose = gst_xlnx_video_src_dispose;

  element_class->change_state = gst_xlnx_video_src_change_state;

  gst_element_class_set_static_metadata (element_class, "Xilinx Video Source",
      "Source/Video", "Reads input from different v4l2 based devices",
      "Naveen Cherukuri <naveenc@xilinx.com>");

  GST_DEBUG_CATEGORY_INIT (gst_xlnx_video_src_debug, "xlnxvideosrc",
      0, "debug category for xlnxvideosrc element");

  gst_element_class_add_static_pad_template (element_class,
      &gst_xlnx_video_src_template);

}

static void
gst_xlnx_video_src_init (GstXlnxVideoSrc * self)
{
  GstElement *video_src;
  guint num_props = 0;
  self->src_type = DEFAULT_VIDEO_SRC_TYPE;
  self->cfg = NULL;
  self->video_src = NULL;
  self->on_error = FALSE;
  self->ghost_pad = gst_ghost_pad_new_no_target ("src", GST_PAD_SRC);
  gst_element_add_pad (GST_ELEMENT (self), self->ghost_pad);
  self->src_idx = -1;
  self->auto_select = TRUE;

  video_src = gst_element_factory_make ("v4l2src", NULL);
  if (video_src) {
    num_props = xlnx_video_src_install_child_properties (self, video_src);
    xlnx_video_src_update_source (self, video_src);
  } else {
    GST_ERROR ("failed to create v4l2src element");
    self->on_error = TRUE;
    return;
  }

  if (!g_object_class_find_property (G_OBJECT_CLASS (GST_ELEMENT_GET_CLASS
              (self)), "src-type")) {
    g_object_class_install_property (G_OBJECT_GET_CLASS (self), num_props,
        g_param_spec_enum ("src-type", "video source type", "Video source type",
            GST_TYPE_XLNX_VIDEO_SRC_TYPE, DEFAULT_VIDEO_SRC_TYPE,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  }
}

static void
gst_xlnx_video_src_dispose (GObject * object)
{
  GstXlnxVideoSrc *self = GST_XLNX_VIDEO_SRC (object);

  if (self->video_src) {
    gst_object_unref (self->video_src);
    self->video_src = NULL;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

void
xlnx_video_src_set_format_cb (GstElement * v4l2src, int fd, GstCaps * caps,
    GstXlnxVideoSrc * self)
{
  struct vlib_config_data *vlib_cfg = self->cfg;
  struct vlib_config config = { };
  GstVideoInfo *vinfo;

  GST_INFO_OBJECT (self, "received prepare-format callback");

  vinfo = gst_video_info_new ();
  if (NULL == vinfo) {
    GST_ERROR_OBJECT (self, "failed to allocated memory...");
    self->on_error = TRUE;
    return;
  }
  GST_INFO_OBJECT (self, "caps configured on pad : %" GST_PTR_FORMAT, caps);
  gst_video_info_from_caps (vinfo, caps);

  // FIXME : handle caps renegotiation as well, by not removing probe and reconfig hw pipe

  /* get width, height and format information */
  vlib_cfg->width_in = GST_VIDEO_INFO_WIDTH (vinfo);
  vlib_cfg->height_in = GST_VIDEO_INFO_HEIGHT (vinfo);
  vlib_cfg->fmt_in = get_v4l2_fourcc (vinfo);
  vlib_cfg->fps.numerator = GST_VIDEO_INFO_FPS_N (vinfo);
  vlib_cfg->fps.denominator = GST_VIDEO_INFO_FPS_D (vinfo);
  GST_INFO_OBJECT (self,
      "caps : width = %d, height = %d, fps = %d/%d, gstformat = %"
      GST_FOURCC_FORMAT, vlib_cfg->width_in, vlib_cfg->height_in,
      vlib_cfg->fps.numerator, vlib_cfg->fps.denominator,
      GST_FOURCC_ARGS (GST_VIDEO_INFO_FORMAT (vinfo)));

  if (0 == vlib_cfg->width_out) {
    vlib_cfg->width_out = vlib_cfg->width_in;
    vlib_cfg->height_out = vlib_cfg->height_in;
    vlib_cfg->fmt_out = vlib_cfg->fmt_in;
  }

  if (vlib_init_gst (vlib_cfg)) {
    GST_ERROR_OBJECT (self, "failed do vlib gst init");
    self->on_error = TRUE;
    return;
  }

  config.vsrc = self->src_idx;
  /* Pass sensor ID for multicam source type */
  if (self->src_type == GST_XLNX_SRC_MIPI_QUAD_VC0 ||
      self->src_type == GST_XLNX_SRC_MIPI_QUAD_VC1 ||
      self->src_type == GST_XLNX_SRC_MIPI_QUAD_VC2 ||
      self->src_type == GST_XLNX_SRC_MIPI_QUAD_VC3) {
    config.type = self->src_type - GST_XLNX_SRC_MIPI_QUAD_VC0;
    GST_INFO_OBJECT (self, "source type : %d", config.type);
  }

  if (vlib_change_mode_gst (&config)) {
    GST_ERROR_OBJECT (self, "failed do vlib change mode gst");
    self->on_error = TRUE;
    return;
  }
  gst_video_info_free (vinfo);
}

static void
xlnx_video_src_update_source (GstXlnxVideoSrc * self, GstElement * video_src)
{
  GstPad *target_pad = NULL;

  self->video_src = video_src;
  g_signal_connect (self->video_src, "prepare-format",
      G_CALLBACK (xlnx_video_src_set_format_cb), self);

  gst_object_ref (self->video_src);
  gst_bin_add (GST_BIN (self), self->video_src);
  target_pad = gst_element_get_static_pad (self->video_src, "src");
  gst_ghost_pad_set_target (GST_GHOST_PAD (self->ghost_pad), target_pad);
  gst_object_unref (target_pad);
}

void
gst_xlnx_video_src_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstXlnxVideoSrc *self = GST_XLNX_VIDEO_SRC (object);

  if (g_object_class_find_property (G_OBJECT_GET_CLASS (self->video_src),
          pspec->name)) {
    self->auto_select = FALSE;
    g_object_set_property (G_OBJECT (self->video_src), pspec->name, value);
  } else if (g_object_class_find_property (G_OBJECT_GET_CLASS (self),
          pspec->name)) {
    self->src_type = (GstXlnxVideoSrcType) g_value_get_enum (value);
  } else {
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

void
gst_xlnx_video_src_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstXlnxVideoSrc *self = GST_XLNX_VIDEO_SRC (object);

  if (g_object_class_find_property (G_OBJECT_GET_CLASS (self->video_src),
          pspec->name)) {
    g_object_get_property (G_OBJECT (self->video_src), pspec->name, value);
  } else if (g_object_class_find_property (G_OBJECT_GET_CLASS (self),
          pspec->name)) {
    g_value_set_enum (value, self->src_type);
  } else {
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

#if 0
static void
list_video_sources (void)
{
  printf ("%16.16s\tID\t%16.16s\n", "VIDEO SOURCE", "VIDEO DEVNODE");

  for (size_t i = 0; i < vlib_video_src_cnt_get (); i++) {
    printf ("%16.16s\t%zu\t%s\n", vlib_video_src_get_display_text_from_id (i),
        i, vlib_video_src_get_vdev_from_id (i));
  }
}
#endif

static void
xlnx_video_src_validate_parameters (GstXlnxVideoSrc * self, char *device_name)
{
  int idx = 0;

  for (idx = 0; idx < vlib_video_src_cnt_get (); idx++) {
    const char *display_text = vlib_video_src_get_display_text_from_id (idx);
    gchar *mod_text = g_ascii_strdown (display_text, strlen (display_text));;

    GST_DEBUG_OBJECT (self, "source type idx = %d and display text = %s", idx,
        display_text);

    if (g_strstr_len (mod_text, strlen (mod_text),
            get_nickname_from_srctype (self->src_type))) {
      const char *device = vlib_video_src_get_vdev_from_id (idx);
      if (g_strcmp0 (device, device_name)) {
        GST_WARNING_OBJECT (self,
            "Warning ::::: issue in device name correct it...\n\n");
        g_object_set (self->video_src, "device", device, NULL);
      } else {
        GST_DEBUG_OBJECT (self, "source type and device name are proper\n\n\n");
      }
      self->src_idx = idx;
      g_free (mod_text);
      break;
    }
    g_free (mod_text);
  }
}

static GstStateChangeReturn
gst_xlnx_video_src_change_state (GstElement * element,
    GstStateChange transition)
{
  GstXlnxVideoSrc *self = GST_XLNX_VIDEO_SRC (element);
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  GST_DEBUG ("changing state: %s => %s",
      gst_element_state_get_name (GST_STATE_TRANSITION_CURRENT (transition)),
      gst_element_state_get_name (GST_STATE_TRANSITION_NEXT (transition)));

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:{
      int idx;
      if (self->on_error) {
        GST_ERROR_OBJECT (self, "failed in initialization");
        return GST_STATE_CHANGE_FAILURE;
      }
      if (self->src_type == GST_XLNX_SRC_NONE) {
        g_warning
            ("Set src-type property of %s to vivid/mipi/hdmi/usbcam/mipi_quad_vcX\n",
            GST_ELEMENT_NAME (self));
        return GST_STATE_CHANGE_FAILURE;
      }
      GST_INFO_OBJECT (self, "chosen source type = %s",
          get_nickname_from_srctype (self->src_type));
      self->cfg =
          (struct vlib_config_data *) calloc (sizeof (struct vlib_config_data),
          1);
      if (NULL == self->cfg) {
        GST_ERROR_OBJECT (self, "failed to allocate memory...");
        return GST_STATE_CHANGE_FAILURE;
      }
      self->cfg->fmt_in = V4L2_PIX_FMT_YUYV;
      self->cfg->fmt_out = V4L2_PIX_FMT_YUYV;
      // FIXME : check whether fmt_int and fmt_out are required ???
      self->cfg->flags |= VLIB_CFG_FLAG_MEDIA_EXIT;

      if (vlib_video_src_init (self->cfg)) {
        GST_ERROR_OBJECT (self, "failed to do videolib source init");
        free (self->cfg);
        return GST_STATE_CHANGE_FAILURE;
      }
#if 0
      list_video_sources ();
#endif

      if (self->auto_select) {
        for (idx = 0; idx < vlib_video_src_cnt_get (); idx++) {
          const char *display_text =
              vlib_video_src_get_display_text_from_id (idx);
          gchar *mod_text =
              g_ascii_strdown (display_text, strlen (display_text));

          GST_DEBUG_OBJECT (self, "source type idx = %d and display text = %s",
              idx, display_text);
          if (g_strstr_len (mod_text, strlen (mod_text),
                  get_nickname_from_srctype (self->src_type))) {
            const char *device = vlib_video_src_get_vdev_from_id (idx);
            if (self->src_type == GST_XLNX_SRC_MIPI_QUAD_VC0
                || self->src_type == GST_XLNX_SRC_MIPI_QUAD_VC1
                || self->src_type == GST_XLNX_SRC_MIPI_QUAD_VC2
                || self->src_type == GST_XLNX_SRC_MIPI_QUAD_VC3) {
              char *devname = strdup (device);
              devname[strlen (devname) - 1] =
                  devname[strlen (devname) - 1] + (self->src_type -
                  GST_XLNX_SRC_MIPI_QUAD_VC0);
              GST_INFO_OBJECT (self,
                  "found chosen source : media_idx = %d and device = %s", idx,
                  devname);
              g_object_set (self->video_src, "device", devname, "io-mode", 4,
                  NULL);
              free (devname);
            } else {
              g_object_set (self->video_src, "device", device, "io-mode", 4,
                  NULL);
              GST_INFO_OBJECT (self,
                  "found chosen source : media idx = %d and device = %s", idx,
                  device);
            }
            self->src_idx = idx;
            g_free (mod_text);
            break;
          }
          g_free (mod_text);
        }
      } else {
        gchar *device_name = NULL;
        g_object_get (self->video_src, "device", &device_name, NULL);
        xlnx_video_src_validate_parameters (self, device_name);
        g_free (device_name);
      }
      break;
    }
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:{
      if (self->cfg) {
        free (self->cfg);
        self->cfg = NULL;
        vlib_video_src_uninit ();
      }
    }
      break;
    default:
      break;
  }
  return ret;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_xlnx_video_src_debug, "xlnxvideosrc", 0,
      "Xilinx video source bin");

  return gst_element_register (plugin, "xlnxvideosrc", GST_RANK_NONE,
      GST_TYPE_XLNX_VIDEO_SRC);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
* in configure.ac and then written into and defined in config.h, but we can
* just set it ourselves here in case someone doesn't use autotools to
* compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
*/
#ifndef PACKAGE
#define PACKAGE "xlnxvideosrc"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, xlnxvideosrc,
    "Xlnx video source bin", plugin_init, "0.1", "LGPL",
    "GStreamer SDX", "http://xilinx.com/")
