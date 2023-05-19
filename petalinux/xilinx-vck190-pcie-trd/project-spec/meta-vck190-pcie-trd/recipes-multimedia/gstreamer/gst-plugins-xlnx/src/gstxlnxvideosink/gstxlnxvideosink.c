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
 * SECTION:xlnxvideosink
 *
 * This sink element is a BIN consists of kmssink as child
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v videotestsrc ! xlnxvideosink sink-type=dp
 * ]|
 * </refsect2>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstxlnxvideosink.h"
#include "video.h"
#include <gst/video/videooverlay.h>

GST_DEBUG_CATEGORY (gst_xlnx_video_sink_debug);
#define GST_CAT_DEFAULT gst_xlnx_video_sink_debug

#define parent_class gst_xlnx_video_sink_parent_class

static void gst_xlnx_video_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * param_spec);
static void gst_xlnx_video_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * param_spec);
static void xlnx_video_sink_update_sink (GstXlnxVideoSink * self,
    GstElement * video_sink);
static GstStateChangeReturn gst_xlnx_video_sink_change_state (GstElement *
    element, GstStateChange transition);
static void gst_xlnx_video_sink_dispose (GObject * object);
static GstStaticPadTemplate gst_xlnx_video_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);
static void gst_xlnx_video_sink_video_overlay_init (GstVideoOverlayInterface *
    iface);

G_DEFINE_TYPE_WITH_CODE (GstXlnxVideoSink, gst_xlnx_video_sink, GST_TYPE_BIN,
    G_IMPLEMENT_INTERFACE (GST_TYPE_VIDEO_OVERLAY,
        gst_xlnx_video_sink_video_overlay_init));

#define GST_XLNX_VIDEO_SINK_GET_PRIVATE(obj)  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GST_TYPE_XLNX_VIDEO_SINK, \
        GstXlnxVideoSinkPrivate))

#define GST_TYPE_XLNX_VIDEO_SINK_TYPE (xlnx_video_sink_get_type ())
GType xlnx_video_sink_get_type (void);

#define XLNX_SINK_DP_BUS_ID "fd4a0000.zynqmp-display"
#define XLNX_SINK_HDMI_BUS_ID "b00c0000.v_mix"

enum
{
  PROP_0,
  PROP_SINK_TYPE,
};

static const char *
get_nickname_from_sinktype (GstXlnxVideoSinkType sinktype)
{
  switch (sinktype) {
    case GST_XLNX_SINK_DP:
      return "dp";
    case GST_XLNX_SINK_HDMI:
      return "hdmi";
    default:
      return NULL;
  }
}

GType
xlnx_video_sink_get_type (void)
{
  static GType xlnx_sink_type = 0;

  if (!xlnx_sink_type) {
    static const GEnumValue sink_types[] = {
      {GST_XLNX_SINK_NONE, "None", "none"},
      {GST_XLNX_SINK_DP, "Display Port", "dp"},
      {GST_XLNX_SINK_HDMI, "HDMI Output", "hdmi"},
      {0, NULL, NULL}
    };
    xlnx_sink_type =
        g_enum_register_static ("GstXlnxVideoSinkType", sink_types);
  }
  return xlnx_sink_type;
}

#define DEFAULT_VIDEO_SINK_TYPE GST_XLNX_SINK_NONE

static void
gst_xlnx_video_sink_set_render_rectangle (GstVideoOverlay * overlay,
    gint x, gint y, gint width, gint height)
{
  GstXlnxVideoSink *self = GST_XLNX_VIDEO_SINK (overlay);
  GstVideoOverlay *child_overlay = GST_VIDEO_OVERLAY (self->video_sink);

  GST_DEBUG_OBJECT (self, "Setting render rectangle to (%d,%d) %dx%d", x, y,
      width, height);

  GST_OBJECT_LOCK (self);
  gst_video_overlay_set_render_rectangle (child_overlay, x, y, width, height);
  GST_OBJECT_UNLOCK (self);
}

static void
gst_xlnx_video_sink_expose (GstVideoOverlay * overlay)
{
  GstXlnxVideoSink *self = GST_XLNX_VIDEO_SINK (overlay);
  GstVideoOverlay *child_overlay = GST_VIDEO_OVERLAY (self->video_sink);

  GST_DEBUG_OBJECT (overlay, "Expose called by application");
  gst_video_overlay_expose (child_overlay);
}

static void
gst_xlnx_video_sink_video_overlay_init (GstVideoOverlayInterface * iface)
{
  iface->expose = gst_xlnx_video_sink_expose;
  iface->set_render_rectangle = gst_xlnx_video_sink_set_render_rectangle;
}

static guint
xlnx_video_sink_install_child_properties (GstXlnxVideoSink * self,
    GstElement * video_sink)
{
  guint num_props = 0;
  GParamSpec **props = NULL;
  GParamSpec **self_props = NULL;
  guint self_num_props = 0;
  int i;

  props =
      g_object_class_list_properties (G_OBJECT_CLASS (GST_ELEMENT_GET_CLASS
          (video_sink)), &num_props);
  self_props =
      g_object_class_list_properties (G_OBJECT_CLASS (GST_ELEMENT_GET_CLASS
          (self)), &self_num_props);
  GST_LOG_OBJECT (self, "number of properties of kmssink = %d and self = %d",
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
    } else if (param_type == G_TYPE_PARAM_VALUE_ARRAY) {        /* ValueArray */
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
    } else if (param_type == GST_TYPE_PARAM_ARRAY_LIST) {
      GstParamSpecArray *src_pspec = GST_PARAM_SPEC_ARRAY_LIST (pspec);

      our_pspec = gst_param_spec_array (g_param_spec_get_name (pspec),
          g_param_spec_get_nick (pspec), g_param_spec_get_blurb (pspec),
          src_pspec->element_spec, pspec->flags);
    } else {
      GST_WARNING_OBJECT ("Unsupported property type %s for property %s",
          g_type_name (param_type), g_param_spec_get_name (pspec));
      G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (video_sink), i, pspec);
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
gst_xlnx_video_sink_class_init (GstXlnxVideoSinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_xlnx_video_sink_set_property;
  gobject_class->get_property = gst_xlnx_video_sink_get_property;
  gobject_class->dispose = gst_xlnx_video_sink_dispose;

  element_class->change_state = gst_xlnx_video_sink_change_state;

  gst_element_class_set_static_metadata (element_class, "Xilinx Video Sink",
      "Sink/Video", "Display video on preferred sink type",
      "Naveen Cherukuri <naveenc@xilinx.com>");

  GST_DEBUG_CATEGORY_INIT (gst_xlnx_video_sink_debug, "xlnxvideosink",
      0, "debug category for xlnxvideosink element");

  gst_element_class_add_static_pad_template (element_class,
      &gst_xlnx_video_sink_template);
}

static void
gst_xlnx_video_sink_init (GstXlnxVideoSink * self)
{
  GstElement *video_sink;
  guint num_props = 0;

  self->sink_type = DEFAULT_VIDEO_SINK_TYPE;
  self->video_sink = NULL;
  self->ghost_pad = gst_ghost_pad_new_no_target ("sink", GST_PAD_SINK);
  gst_element_add_pad (GST_ELEMENT (self), self->ghost_pad);
  self->on_error = FALSE;
  self->is_fullscreen_overlay = TRUE;
  self->is_autoselect_driver = TRUE;

  video_sink = gst_element_factory_make ("kmssink", NULL);
  if (video_sink) {
    num_props = xlnx_video_sink_install_child_properties (self, video_sink);
    xlnx_video_sink_update_sink (self, video_sink);
  } else {
    GST_ERROR_OBJECT (self, "failed to create kmssink element");
    self->on_error = TRUE;
  }

  if (!g_object_class_find_property (G_OBJECT_CLASS (GST_ELEMENT_GET_CLASS
              (self)), "sink-type")) {
    g_object_class_install_property (G_OBJECT_GET_CLASS (self), num_props,
        g_param_spec_enum ("sink-type", "video sink type", "Video sink type",
            GST_TYPE_XLNX_VIDEO_SINK_TYPE, DEFAULT_VIDEO_SINK_TYPE,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  }
}

static void
gst_xlnx_video_sink_dispose (GObject * object)
{
  GstXlnxVideoSink *self = GST_XLNX_VIDEO_SINK (object);

  if (self->video_sink) {
    gst_object_unref (self->video_sink);
    self->video_sink = NULL;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
xlnx_video_sink_set_dp_properties (GstXlnxVideoSink * self)
{
  if (self->is_autoselect_driver) {
    g_object_set (self->video_sink, "bus-id", XLNX_SINK_DP_BUS_ID, NULL);
  }
  g_object_set (self->video_sink, "fullscreen-overlay",
      self->is_fullscreen_overlay, NULL);
}

static void
xlnx_video_sink_set_hdmitx_properties (GstXlnxVideoSink * self)
{
  if (self->is_autoselect_driver) {
    GST_INFO_OBJECT (self, "bus-id picked automatically...");
    g_object_set (self->video_sink, "bus-id", XLNX_SINK_HDMI_BUS_ID, NULL);
  }
}

static void
xlnx_video_sink_update_sink (GstXlnxVideoSink * self, GstElement * video_sink)
{
  GstPad *target_pad = NULL;

  if (self->video_sink) {
    /* remove ghost pad target */
    gst_ghost_pad_set_target (GST_GHOST_PAD (self->ghost_pad), NULL);

    /* remove old sink */
    gst_bin_remove (GST_BIN (self), self->video_sink);
    gst_object_unref (self->video_sink);
  }
  // FIMXE : check IS_SINK
  self->video_sink = video_sink;
  if (self->video_sink == NULL) {
    GST_ERROR_OBJECT (self, "video sink element is NULL..");
    return;
  }

  gst_object_ref (self->video_sink);
  gst_bin_add (GST_BIN (self), self->video_sink);
  target_pad = gst_element_get_static_pad (self->video_sink, "sink");
  gst_ghost_pad_set_target (GST_GHOST_PAD (self->ghost_pad), target_pad);
  gst_object_unref (target_pad);
}

void
gst_xlnx_video_sink_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstXlnxVideoSink *self = GST_XLNX_VIDEO_SINK (object);

  if (g_object_class_find_property (G_OBJECT_GET_CLASS (self->video_sink),
          pspec->name)) {
    if (!g_strcmp0 (pspec->name, "driver-name")
        || !g_strcmp0 (pspec->name, "bus-id")) {
      if (g_value_get_string (value)) {
        self->is_autoselect_driver = FALSE;
      }
      GST_DEBUG_OBJECT (self,
          "don't do auto selection of bus-id or driver-name... property name = %s and value = %s",
          pspec->name, g_value_get_string (value));
    } else if (!g_strcmp0 (pspec->name, "fullscreen-overlay")) {
      self->is_fullscreen_overlay = g_value_get_boolean (value);
    }
    g_object_set_property (G_OBJECT (self->video_sink), pspec->name, value);
  } else if (g_object_class_find_property (G_OBJECT_GET_CLASS (self),
          pspec->name)) {
    self->sink_type = (GstXlnxVideoSinkType) g_value_get_enum (value);
  } else {
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

void
gst_xlnx_video_sink_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstXlnxVideoSink *self = GST_XLNX_VIDEO_SINK (object);

  if (g_object_class_find_property (G_OBJECT_GET_CLASS (self->video_sink),
          pspec->name)) {
    g_object_get_property (G_OBJECT (self->video_sink), pspec->name, value);
  } else if (g_object_class_find_property (G_OBJECT_GET_CLASS (self),
          pspec->name)) {
    g_value_set_enum (value, self->sink_type);
  } else {
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static GstStateChangeReturn
gst_xlnx_video_sink_change_state (GstElement * element,
    GstStateChange transition)
{
  GstXlnxVideoSink *self = GST_XLNX_VIDEO_SINK (element);
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  GST_DEBUG ("changing state: %s => %s",
      gst_element_state_get_name (GST_STATE_TRANSITION_CURRENT (transition)),
      gst_element_state_get_name (GST_STATE_TRANSITION_NEXT (transition)));

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
    {
      struct vlib_config_data *config = NULL;
      if (self->on_error) {
        GST_ERROR_OBJECT (self, "failed in initialization");
        return GST_STATE_CHANGE_FAILURE;
      }
      if (self->sink_type == GST_XLNX_SINK_NONE) {
        g_warning ("Set sink-type property of %s to dp/hdmi\n",
            GST_ELEMENT_NAME (self));
        return GST_STATE_CHANGE_FAILURE;
      }
      GST_INFO_OBJECT (self, "Chosen sink type = %s",
          get_nickname_from_sinktype (self->sink_type));

      if (GST_XLNX_SINK_DP == self->sink_type) {
        xlnx_video_sink_set_dp_properties (self);
      } else if (GST_XLNX_SINK_HDMI == self->sink_type) {
        xlnx_video_sink_set_hdmitx_properties (self);
      }

      config =
          (struct vlib_config_data *) calloc (sizeof (struct vlib_config_data),
          1);
      config->fmt_in = V4L2_PIX_FMT_YUYV;
      config->fmt_out = V4L2_PIX_FMT_YUYV;
      config->display_id = self->sink_type;
      config->flags |= VLIB_CFG_FLAG_MEDIA_EXIT;

      if (vlib_platform_setup (config)) {
        GST_ERROR_OBJECT (self, "failed to do vlib platform setup");
        return GST_STATE_CHANGE_FAILURE;
      } else {
        GST_INFO_OBJECT (self, "vlib platform setup successful...");
      }
      free (config);
    }
      break;
    default:
      break;
  }

  ret = GST_CALL_PARENT_WITH_DEFAULT (GST_ELEMENT_CLASS, change_state,
      (element, transition), GST_STATE_CHANGE_SUCCESS);

  switch (transition) {
    default:
      break;
  }

  return ret;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_xlnx_video_sink_debug, "xlnxvideosink", 0,
      "Xilinx video sink bin");

  return gst_element_register (plugin, "xlnxvideosink", GST_RANK_NONE,
      GST_TYPE_XLNX_VIDEO_SINK);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
* in configure.ac and then written into and defined in config.h, but we can
* just set it ourselves here in case someone doesn't use autotools to
* compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
*/
#ifndef PACKAGE
#define PACKAGE "xlnxvideosink"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, xlnxvideosink,
    "Xlnx video sink bin", plugin_init, "0.1", "LGPL",
    "GStreamer SDX", "http://xilinx.com/")
