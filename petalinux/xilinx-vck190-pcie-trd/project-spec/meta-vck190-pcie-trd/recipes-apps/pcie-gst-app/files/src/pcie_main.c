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

#include <pcie_main.h>
#include <pcie_src.h>
#include <pcie_sink.h>
#include <dpu_pipe.h>

static gchar* aitask = (gchar*)"facedetect";
App s_app;
GST_DEBUG_CATEGORY (pcie_gst_app_debug);

#define GST_CAT_DEFAULT pcie_gst_app_debug

static gboolean bus_message (GstBus *bus, GstMessage *message, App *app)
{
    GError* err   = NULL;
    gchar*  debug = NULL;

    switch (GST_MESSAGE_TYPE (message)) {

        case GST_MESSAGE_INFO:
            gst_message_parse_info (message, &err, &debug);
            if (debug) {
                GST_INFO ("INFO: %s", debug);
                g_free (debug);
            }
            if (err)
                g_error_free (err);
            break;

        case GST_MESSAGE_ERROR:
            gst_message_parse_info (message, &err, &debug);
            if(err && message) {
                GST_ERROR ("Received ERROR from %s: %s",
                GST_MESSAGE_SRC_NAME (message), err->message);
                g_error_free (err);
            }
            if (debug) {
                GST_ERROR ("ERROR: %s", debug);
                g_free (debug);
            }
            if (app->loop && g_main_loop_is_running(app->loop)) {
                g_main_loop_quit (app->loop);
                GST_ERROR ("Quitting the loop");
                g_main_loop_unref (app->loop);
                app->loop = NULL;
            }
            break;

        case GST_MESSAGE_EOS:
            if (g_main_loop_is_running (app->loop)) {
                if (app->loop) {
                    g_main_loop_quit (app->loop);
                    GST_DEBUG ("Quitting the loop");
                    g_main_loop_unref (app->loop);
                    app->loop = NULL;
                }
            }
            break;

        default:
            break;
    }

    return TRUE;
}

static void read_write_transfer_done (App* app)
{
    gint ret = 0;

    /* Avoid sending read transfer done in mipi use-case */
    if (app->h_param.usecase > VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS) {
        ret = pcie_set_read_transfer_done(app->fd);
        if (ret >= 0)
            GST_DEBUG ("set read transfer done");
    }

    /* Send write transfer done for all use-case types */
    ret = pcie_set_write_transfer_done(app->fd);
    if (ret >= 0)
        GST_DEBUG ("set write transfer done");

    /* Delay between set and clear r/w done registers */
    sleep (RW_DONE_SET_AND_CLEAR_DELAY);

    /* Avoid clearing read transfer done in mipi use-case */
    if (app->h_param.usecase > VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS) {
        ret = pcie_clr_read_transfer_done(app->fd);
        if (ret >= 0)
            GST_DEBUG ("clear read transfer done");
    }

    /* Clear write transfer done for all use-case types */
    ret = pcie_clr_write_transfer_done(app->fd);
    if (ret >= 0)
        GST_DEBUG ("clear write transfer done");
}

static GstPadProbeReturn appsink_query_cb (GstPad *pad G_GNUC_UNUSED,
             GstPadProbeInfo *info, gpointer user_data G_GNUC_UNUSED)
{
    GstQuery *query = info->data;

    if (GST_QUERY_TYPE (query) != GST_QUERY_ALLOCATION)
        return GST_PAD_PROBE_OK;

    gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);

    return GST_PAD_PROBE_HANDLED;
}

static guint64 get_export_fd_size(guint64 framesize)
{
    guint   diff     = 0;
    guint   rem      = 0;
    guint   pagesize = 0;
    guint64 expfd_sz = 0;

    /* Get page size */
    pagesize = sysconf(_SC_PAGESIZE);

    /* Get reminder */
    rem = framesize % pagesize;

    if(rem != 0) {
        /* Get difference that we need to add in framesize */
        diff = pagesize - rem;

        /* Update the export fd size, so that the resultant framesize will be in
           multiple of the pagesize */
        expfd_sz = framesize + diff;
    }
    else
        expfd_sz = framesize;

    return expfd_sz;
}

static gint set_host_parameters(App *app)
{
    gint ret = 0;

    /* Get usecase type */
    ret = pcie_get_usecase_type(app->fd, &(app->h_param.usecase));
    if (ret < 0) {
        GST_ERROR ("Failed to get usecase type");
        return PCIE_GST_APP_FAIL;
    }
    if ((app->h_param.usecase == VGST_USECASE_TYPE_NONE) ||
        (app->h_param.usecase >= VGST_USECASE_TYPE_MAX)) {
        GST_ERROR ("Provided usecase type is not supported, received "
                   "usecase type - %u", app->h_param.usecase);
        return PCIE_GST_APP_FAIL;
    }
    GST_DEBUG ("Usecase type is %d",app->h_param.usecase);

    /* Get input file length */
    if (app->h_param.usecase >= VGST_USECASE_TYPE_APPSRC_TO_HOST ) {
        ret = pcie_get_file_length(app->fd, &(app->h_param.length));
        if(ret < 0) {
            GST_ERROR ("Failed to get file length");
            return PCIE_GST_APP_FAIL;
        }
        GST_DEBUG ("Input file length is %lu",app->h_param.length);
    }

    /* Get input resolution */
    ret = pcie_get_input_resolution (app->fd, &app->h_param.input_res);
    if (ret < 0) {
        GST_ERROR ("Failed to get input resolution");
        return PCIE_GST_APP_FAIL;
    }
    GST_DEBUG ("Resolution is %d x %d", app->h_param.input_res.width,
                                        app->h_param.input_res.height);

    /* Get the input fps */
    ret = pcie_get_fps(app->fd, &(app->h_param.fps));
    if (ret < 0) {
        GST_ERROR ("Failed to get fps");
        return PCIE_GST_APP_FAIL;
    }
    GST_DEBUG ("FPS is %d",app->h_param.fps);

    /* Get filter-preset */
    if (app->h_param.usecase != VGST_USECASE_TYPE_APPSRC_TO_HOST_BYPASS &&  
    	app->h_param.usecase != VGST_USECASE_TYPE_APPSRC_TO_KMSSINK_BYPASS && 
    	app->h_param.usecase != VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS &&
	app->h_param.usecase != VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK &&
	app->h_param.usecase != VGST_USECASE_TYPE_MIPISRC_DPU_TO_HOST) {
        ret = pcie_get_filter_type (app->fd, &(app->h_param.filter_preset));
        if (ret < 0) {
            GST_ERROR ("Failed to get filter type");
            return PCIE_GST_APP_FAIL;
        }
        if (app->h_param.filter_preset >= VGST_FILTER_PRESET_MAX) {
            GST_ERROR ("Provided filter preset is not supported");
            return PCIE_GST_APP_FAIL;
        }
        GST_DEBUG ("PCIe Filter Preset is %d",app->h_param.filter_preset);
    }

    /* Get format type */
    /* NOTE: Only YUY2 format is supported currently */
    app->h_param.input_format = VGST_FORMAT_YUY2;
    if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK) {
    	app->h_param.input_format = VGST_FORMAT_BGR;
	app->yuv_frame_size = app->h_param.input_res.width *  \
                          app->h_param.input_res.height * BGR_MULTIPLIER;
    } else {
    	app->yuv_frame_size = app->h_param.input_res.width *  \
                          app->h_param.input_res.height * YUY2_MULTIPLIER;
    }

    GST_DEBUG ("YUV Frame size is %lu",app->yuv_frame_size);

    /* Get Kernel mode */
    ret = pcie_get_kernel_mode(app->fd, &(app->h_param.kernel_mode));
    if (ret < 0) {
        GST_ERROR ("Failed to get kernel mode");
        return PCIE_GST_APP_FAIL;
    }
    if (app->h_param.kernel_mode >= VGST_FILTER_MODE_MAX) {
        GST_ERROR ("Provided kernel mode is not supported");
        return PCIE_GST_APP_FAIL;
    }
    GST_DEBUG ("Kernel mode is %d",app->h_param.kernel_mode);

    Set_config_info(app->h_param.input_res.width,
		    app->h_param.input_res.height,
		    aitask);

    return ret;
}

static char * getfilterstring(int index,char *strbuf) {
   const char *str1 = "{\"filter_preset\":\"";
   const char *str2 = "{\"filter_preset\" : \"";
   if(index > 1){
   strbuf = (char *)malloc(strlen(str1) + strlen(filter_presets[index])+1);
   sprintf(strbuf, "%s%s\"\}", str1, filter_presets[index] );
   }
   else {
   strbuf = (char *)malloc(strlen(str2) + strlen(filter_presets[index])+1);
   sprintf(strbuf, "%s%s\"\}", str2, filter_presets[index] );
   }
   return strbuf;
}

static gint gst_set_elements (App *app)
{
    gint ret = 0;

    app->pipeline       = gst_pipeline_new          ("pipeline");
    app->inputsrc       = gst_element_factory_make  ("v4l2src",         NULL);
    app->capsfilter     = gst_element_factory_make  ("capsfilter",      NULL);
    app->pciesrc        = gst_element_factory_make  ("appsrc",          NULL);
    app->pciesink       = gst_element_factory_make  ("appsink",         NULL);
    app->hdmisink       = gst_element_factory_make  ("kmssink",         NULL);
    app->vvas_xfilter    = gst_element_factory_make  ("vvas_xfilter",     NULL);
    app->perf           = gst_element_factory_make  ("perf",            NULL);
    if (!app->pipeline || !app->inputsrc  || !app->capsfilter  || \
        !app->pciesrc  || !app->pciesink  || !app->vvas_xfilter || \
	!app->hdmisink || !app->perf) {
      GST_ERROR ("Failed to create required GStreamer elements");
      return PCIE_GST_APP_FAIL;
    }
    GST_DEBUG ("Created all required GStreamer elements");

    return ret;
}

static void gst_reset_elements (App *app)
{
    gst_object_unref (GST_OBJECT (app->pipeline));
    gst_object_unref (GST_OBJECT (app->inputsrc));
    gst_object_unref (GST_OBJECT (app->capsfilter));
    gst_object_unref (GST_OBJECT (app->pciesrc));
    gst_object_unref (GST_OBJECT (app->pciesink));
    gst_object_unref (GST_OBJECT (app->hdmisink));
    gst_object_unref (GST_OBJECT (app->vvas_xfilter));
    gst_object_unref (GST_OBJECT (app->perf));
    GST_DEBUG ("Released GStreamer elements");
}

static void set_property (App *app)
{
    GstCaps* srcCaps = NULL;
    char * preset,*strbuf;	
    if (app->h_param.usecase >= VGST_USECASE_TYPE_APPSRC_TO_HOST) {
        GST_DEBUG ("Setting up appsrc plugin");
        g_object_set (G_OBJECT (app->pciesrc),                      \
                "stream-type", GST_APP_STREAM_TYPE_STREAM,          \
                "format",      GST_FORMAT_TIME,                     \
                "is-live",     TRUE,                                \
                "block",       TRUE,                                \
                "max-bytes",   app->yuv_frame_size,                 \
                NULL);
        srcCaps = gst_caps_new_simple ("video/x-raw",
                "format",     G_TYPE_STRING,                        \
                              app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK ? "BGR" : "YUY2",  \
                "width",      G_TYPE_INT,                           \
                              app->h_param.input_res.width,         \
                "height",     G_TYPE_INT,                           \
                              app->h_param.input_res.height,        \
                "framerate",  GST_TYPE_FRACTION,                    \
                              app->h_param.fps,                     \
                              MAX_FRAME_RATE_DENOM, NULL);
        GST_DEBUG ("New Caps for appsrc %" GST_PTR_FORMAT, srcCaps);
        g_object_set (G_OBJECT (app->pciesrc),  "caps",  srcCaps, NULL);
        gst_caps_unref (srcCaps);
    }
    else if (app->h_param.usecase <= VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS) {
        GST_DEBUG ("Setting up v4l2src plugin");
        g_object_set (G_OBJECT(app->inputsrc),                      \
                "io-mode",   VGST_V4L2_IO_MODE_DMABUF_EXPORT,       \
                "device",    INPUT_SRC,                             \
                NULL);
        srcCaps  = gst_caps_new_simple ("video/x-raw",              \
                "width",     G_TYPE_INT,                            \
                             app->h_param.input_res.width,          \
                "height",    G_TYPE_INT,                            \
                             app->h_param.input_res.height,         \
                "format",    G_TYPE_STRING,                         \
                             (app->h_param.usecase == VGST_USECASE_TYPE_MIPISRC_DPU_TO_HOST)? "BGR" : VIDEOPARSE_FORMAT_YUY2,                \
                "framerate", GST_TYPE_FRACTION,                     \
                             app->h_param.fps,                      \
                             MAX_FRAME_RATE_DENOM,                  \
                NULL);
        GST_DEBUG ("New Caps for capsfilter %" GST_PTR_FORMAT, srcCaps);
        g_object_set (G_OBJECT (app->capsfilter),  "caps",  srcCaps, NULL);
        gst_caps_unref (srcCaps);
    }

    if (app->h_param.usecase < VGST_USECASE_TYPE_APPSRC_TO_KMSSINK) {
    	/* Configure appsink */
    	g_object_set (G_OBJECT (app->pciesink), \
            	"emit-signals", TRUE,           \
            	"sync",         FALSE,          \
            	"async",        FALSE,          \
            	NULL);
    }
    else if(app->h_param.usecase >= VGST_USECASE_TYPE_APPSRC_TO_KMSSINK) {
    	/* Configure kmssink */
	g_object_set (G_OBJECT (app->hdmisink), \
            	"driver-name", "xlnx",           \
            	"plane-id",    (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK)? 36 : 38, \
            	"sync",        FALSE,          \
            	NULL);
    }
    if(app->h_param.usecase != VGST_USECASE_TYPE_APPSRC_TO_HOST_BYPASS &&
       app->h_param.usecase != VGST_USECASE_TYPE_APPSRC_TO_KMSSINK_BYPASS && 
       app->h_param.usecase != VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS && 
       app->h_param.usecase != VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK &&
       app->h_param.usecase != VGST_USECASE_TYPE_MIPISRC_DPU_TO_HOST) {
        /* Configure vvas_xfilter parameters */
        GST_DEBUG ("Setting up filter2d plugin");
        g_object_set (G_OBJECT (app->vvas_xfilter),              \
                "kernels-config", "/usr/share/vvas/vck190-pcie-trd/kernel_xfilter2d_pl.json",   \
                NULL);
	preset = getfilterstring(app->h_param.filter_preset,strbuf);
	g_object_set (G_OBJECT (app->vvas_xfilter),              \
                "dynamic-config",preset,   \
                NULL);
    }
    free(strbuf);
    strbuf = NULL;
    preset = NULL;
}

static gint create_pipeline (App *app)
{
    gint ret = 0;

    if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK) {
        /* pciesrc -> dpu -> hdmisink -> displayonmonitor */
        GError *err = NULL; 
        char* pipestr = Get_pipeline_str(app->h_param.usecase);

        app->dpubin = gst_parse_bin_from_description (pipestr, TRUE, &err);
        gst_bin_add_many (GST_BIN (app->pipeline), app->pciesrc, app->perf, app->dpubin, app->hdmisink, NULL);
        if (gst_element_link_many (app->pciesrc, app->dpubin, app->perf, app->hdmisink, NULL) != TRUE) {
            GST_ERROR ("Error linking pciesrc --> perf --> dpu --> "                \
                       "hdmisink pipeline");
            ret = PCIE_GST_APP_FAIL;
        } else {
            GST_DEBUG ("Linked pciesrc --> perf --> dpu --> hdmisink "              \
                       "pipeline successfully");
        }
    }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_MIPISRC_DPU_TO_HOST) {
        GError *err = NULL; 
        char* pipestr = Get_pipeline_str(app->h_param.usecase);
        app->dpubin = gst_parse_bin_from_description (pipestr, TRUE, &err);

	gst_bin_add_many (GST_BIN (app->pipeline), app->inputsrc,       \
	        app->capsfilter, app->dpubin, app->perf, app->pciesink, \
	        NULL);
	if (gst_element_link_many (app->inputsrc, app->capsfilter, app->dpubin,  app->perf, app->pciesink \
	        , NULL) != TRUE) {
	   GST_ERROR ("Error linking v4l2src --> capsfilter --> dpu"      \
	   	    " --> perf --> pciesink pipeline");
	   ret = PCIE_GST_APP_FAIL;
	} else{
	    GST_DEBUG ("Linked v4l2src --> capsfilter --> dpu" \
	            " --> perf --> pciesink successfully");
	}

    } 
    else if (app->h_param.usecase == VGST_USECASE_TYPE_MIPISRC_TO_HOST) {
        /* mipi -> filter2d -> pciesink -> displayonhost */
        gst_bin_add_many (GST_BIN (app->pipeline), app->inputsrc,       \
                app->capsfilter, app->vvas_xfilter, app->perf,           \
                app->pciesink, NULL);
        if (gst_element_link_many (app->inputsrc, app->capsfilter,      \
                app->vvas_xfilter, app->perf, app->pciesink, NULL)       \
                != TRUE) {
            GST_ERROR ("Error linking v4l2src --> capsfilter --> "      \
                       "vvas_xfilter --> perf --> pciesink pipeline");
            ret = PCIE_GST_APP_FAIL;
        } else{
            GST_DEBUG ("Linked v4l2src --> capsfilter --> vvas_xfilter"  \
                       " --> perf --> pciesink successfully");
        }
    }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS) {
	gst_bin_add_many (GST_BIN (app->pipeline), app->inputsrc,       \
	        app->capsfilter, app->perf,app->pciesink, 		\
	        NULL);
	if (gst_element_link_many (app->inputsrc, app->capsfilter, app->perf, \
	        app->pciesink, NULL) != TRUE) {
	   GST_ERROR ("Error linking v4l2src --> capsfilter --> "      \
	   	    " --> perf --> pciesink pipeline");
	   ret = PCIE_GST_APP_FAIL;
	} else{
	    GST_DEBUG ("Linked v4l2src --> capsfilter --> " \
	            " --> perf --> pciesink successfully");
		}
	}
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_TO_HOST) {
        /* pciesrc -> filter2d -> pciesink -> displayonhost */
        gst_bin_add_many (GST_BIN (app->pipeline), app->pciesrc,        \
                app->vvas_xfilter, app->perf, app->pciesink, NULL);
        if (gst_element_link_many (app->pciesrc, app->vvas_xfilter,      \
                app->perf, app->pciesink, NULL) != TRUE) {
            GST_ERROR ("Error linking pciesrc --> vvas_xfilter --> "     \
                       "perf --> pciesink pipeline");
            ret = PCIE_GST_APP_FAIL;
        } else {
            GST_DEBUG ("Linked pciesrc --> vvas_xfilter --> perf --> "   \
                       "pciesink pipeline successfully");
        }
    }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_TO_HOST_BYPASS) {
        /* pciesrc -> pciesink -> displayonhost */
        gst_bin_add_many (GST_BIN (app->pipeline), app->pciesrc,            \
                app->perf, app->pciesink, NULL);
        if (gst_element_link_many (app->pciesrc, app->perf,                 \
                app->pciesink, NULL) != TRUE) {
            GST_ERROR ("Error linking pciesrc --> perf --> "                \
                       "pciesink pipeline");
            ret = PCIE_GST_APP_FAIL;
        } else {
            GST_DEBUG ("Linked pciesrc --> perf --> pciesink "              \
                       "pipeline successfully");
        }
     }
     else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_TO_KMSSINK) {
        /* pciesrc -> hdmisink -> filter2d -> displayonmonitor */
        gst_bin_add_many (GST_BIN (app->pipeline), app->pciesrc,            \
                app->vvas_xfilter, app->perf, app->hdmisink, NULL);
        if (gst_element_link_many (app->pciesrc, app->perf,                 \
                app->vvas_xfilter, app->hdmisink, NULL) != TRUE) {
            GST_ERROR ("Error linking pciesrc --> perf --> "                \
                       "hdmisink pipeline");
            ret = PCIE_GST_APP_FAIL;
        } else {
            GST_DEBUG ("Linked pciesrc --> vvas_xfilter -->perf  --> hdmisink "              \
                       "pipeline successfully");
        }
     }
     else if (app->h_param.usecase >= VGST_USECASE_TYPE_APPSRC_TO_KMSSINK_BYPASS) {
        /* pciesrc -> hdmisink -> displayonmonitor */
        gst_bin_add_many (GST_BIN (app->pipeline), app->pciesrc,            \
                app->perf, app->hdmisink, NULL);
        if (gst_element_link_many (app->pciesrc, app->perf,                 \
                app->hdmisink, NULL) != TRUE) {
            GST_ERROR ("Error linking pciesrc --> perf --> "                \
                       "hdmisink pipeline");
            ret = PCIE_GST_APP_FAIL;
        } else {
            GST_DEBUG ("Linked pciesrc --> perf --> hdmisink "              \
                       "pipeline successfully");
        }
    }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK) {
        /* pciesrc -> hdmisink -> displayonmonitor */
        GError *err = NULL; 
        char* pipestr = Get_pipeline_str(app->h_param.usecase);
        app->dpubin = gst_parse_bin_from_description (pipestr, TRUE, &err);
        gst_bin_add_many (GST_BIN (app->pipeline), app->pciesrc, app->dpubin, NULL);
        if (gst_element_link_many (app->pciesrc, app->dpubin, NULL) != TRUE) {
            GST_ERROR ("Error linking pciesrc --> perf --> "                \
                       "hdmisink pipeline");
            ret = PCIE_GST_APP_FAIL;
        } else {
            GST_DEBUG ("Linked pciesrc --> perf --> hdmisink "              \
                       "pipeline successfully");
        }
    }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_MIPISRC_DPU_TO_HOST) {
        /* pciesrc -> hdmisink -> displayonmonitor */
        GError *err = NULL; 
        char* pipestr = Get_pipeline_str(app->h_param.usecase);
        app->dpubin = gst_parse_bin_from_description (pipestr, TRUE, &err);

        gst_bin_add_many (GST_BIN (app->pipeline), app->inputsrc,
                app->capsfilter, app->dpubin,app->pciesink,
                NULL);
        if (gst_element_link_many (app->inputsrc,
                app->capsfilter, app->dpubin, app->pciesink, NULL) != TRUE) {
            GST_ERROR ("Error linking inputsrc --> dpu --> "                \
                       "host pipeline");
            ret = PCIE_GST_APP_FAIL;
        } else {
            GST_DEBUG ("Linked inputsrc --> dpu --> host "              \
                       "pipeline successfully");
        }
    }
    return ret;
}

static void destroy_pipeline (App *app)
{
    gst_object_ref (app->pipeline);
    gst_object_ref (app->perf);

    if (app->h_param.usecase == VGST_USECASE_TYPE_MIPISRC_TO_HOST) {
        /* mipi -> filter2d -> pciesink -> displayonhost */
        gst_element_unlink_many (app->inputsrc, app->capsfilter,            \
                app->vvas_xfilter, app->perf, app->pciesink, NULL);
        gst_object_ref (app->inputsrc);
        gst_object_ref (app->capsfilter);
        gst_object_ref (app->vvas_xfilter);
        gst_object_ref (app->pciesink);
        gst_bin_remove_many (GST_BIN (app->pipeline), app->inputsrc,        \
                app->capsfilter, app->vvas_xfilter, app->perf,               \
                app->pciesink, NULL);
        GST_DEBUG ("Destroyed v4l2src --> capsfilter --> vvas_xfilter"       \
                   " --> perf --> pciesink successfully");
    }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS) {
        /* mipi -> filter2d -> pciesink -> displayonhost */
        gst_element_unlink_many (app->inputsrc, app->capsfilter,            \
                 app->perf, app->pciesink, NULL);
        gst_object_ref (app->inputsrc);
        gst_object_ref (app->capsfilter);
        gst_object_ref (app->pciesink);
        gst_bin_remove_many (GST_BIN (app->pipeline), app->inputsrc,        \
                app->capsfilter, app->perf,app->pciesink, NULL);
        GST_DEBUG ("Destroyed v4l2src --> capsfilter --> \
                    --> perf --> pciesink successfully");
    }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_TO_HOST) {
        /* pciesrc -> filter2d -> pciesink -> displayonhost */
        gst_element_unlink_many (app->pciesrc, app->vvas_xfilter,            \
                app->perf, app->pciesink, NULL);
        gst_object_ref (app->pciesrc);
        gst_object_ref (app->vvas_xfilter);
        gst_object_ref (app->pciesink);
        gst_bin_remove_many (GST_BIN (app->pipeline), app->pciesrc,         \
                app->vvas_xfilter, app->perf, app->pciesink, NULL);
        GST_DEBUG ("Destroyed pciesrc --> vvas_xfilter --> perf --> "        \
                   "pciesink pipeline successfully");
    }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_TO_HOST_BYPASS) {
        /* pciesrc -> pciesink -> displayonhost */
        gst_element_unlink_many (app->pciesrc, app->perf, app->pciesink, NULL);
        gst_object_ref (app->pciesrc);
        gst_object_ref (app->pciesink);
        gst_bin_remove_many (GST_BIN (app->pipeline), app->pciesrc,         \
                app->perf, app->pciesink, NULL);
        GST_DEBUG ("Destroyed pciesrc --> perf --> pciesink "               \
                   "pipeline successfully");
     }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_TO_KMSSINK) {
        /* pciesrc -> hdmisink -> displayonmonitor */
        gst_element_unlink_many (app->pciesrc, app->perf, app->vvas_xfilter, \
	app->hdmisink, NULL);
        gst_object_ref (app->pciesrc);
        gst_object_ref (app->vvas_xfilter);
        gst_object_ref (app->hdmisink);
        gst_bin_remove_many (GST_BIN (app->pipeline), app->pciesrc,app->vvas_xfilter,\
                app->perf, app->hdmisink, NULL);
        GST_DEBUG ("Destroyed pciesrc --> vvas_xfilter --> perf --> hdmisink "               \
                   "pipeline successfully");
		   }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_TO_KMSSINK_BYPASS) {
        /* pciesrc -> hdmisink -> displayonmonitor */
        gst_element_unlink_many (app->pciesrc, app->perf, app->hdmisink, NULL);
        gst_object_ref (app->pciesrc);
        gst_object_ref (app->hdmisink);
        gst_bin_remove_many (GST_BIN (app->pipeline), app->pciesrc,         \
                app->perf, app->hdmisink, NULL);
        GST_DEBUG ("Destroyed pciesrc --> perf --> hdmisink "               \
                   "pipeline successfully");
		   }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_DPU_TO_KMSSINK) {
        /* pciesrc -> hdmisink -> displayonmonitor */
        gst_element_unlink_many (app->pciesrc, app->perf, app->hdmisink, NULL);
        gst_object_ref (app->pciesrc);
        gst_object_ref (app->hdmisink);
        gst_bin_remove_many (GST_BIN (app->pipeline), app->pciesrc,         \
                app->perf, app->hdmisink, NULL);
        GST_DEBUG ("Destroyed pciesrc --> perf --> hdmisink "               \
                   "pipeline successfully");
		   }
    else if (app->h_param.usecase == VGST_USECASE_TYPE_APPSRC_TO_KMSSINK_BYPASS) {
        /* pciesrc -> hdmisink -> displayonmonitor */
        gst_element_unlink_many (app->pciesrc, app->perf, app->hdmisink, NULL);
        gst_object_ref (app->pciesrc);
        gst_object_ref (app->hdmisink);
        gst_bin_remove_many (GST_BIN (app->pipeline), app->pciesrc,         \
                app->perf, app->hdmisink, NULL);
        GST_DEBUG ("Destroyed pciesrc --> perf --> hdmisink "               \
                   "pipeline successfully");
		   }

}

static gpointer host_app_reg_read (gpointer data)
{
    gint       ret            = 0;
    guint      stop_mipi_feed = 0;
    GstEvent*  event          = NULL;
    App*       app            = (App*) data;

    GST_DEBUG ("starting hostapp register read thread");

    while(!app->exit_thread) {

        /* Check for stop mipi feed signal only when mipi use-case is running */
        if ((app->loop) &&
            (app->h_param.usecase <= VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS)) {

            stop_mipi_feed = 0;
            ret = pcie_read_stop_mipi_feed (app->fd, &stop_mipi_feed);
            if (ret < 0) {
                GST_WARNING ("Failed to read stop mipi feed signal");
            }

            if (stop_mipi_feed) {
                GST_DEBUG ("Stop mipi feed signal received");
                if (app->loop && g_main_loop_is_running (app->loop)) {
                    GST_DEBUG ("Quitting the playback");
                    event = gst_event_new_eos();
                    if (event) {
                        if (gst_element_send_event (app->pipeline, event)) {
                            GST_DEBUG ("Sent EOS event to quit mipi pipeline");
                        } else {
                            gst_event_unref (event);
                            GST_ERROR ("Failed to send EOS event to quit mipi "
                                       "pipeline");
                        }
                    }
                }
            }
        }

        /* Move to IDLE mode until timeout */
        sleep (HOST_APP_REG_READ_TIMEOUT);
    }
    
    GST_DEBUG("Exit thread is set, quitting hostapp register read thread");
    g_thread_exit(0);
    return NULL;
}

static GOptionEntry entries[] =
{
    { "aitask", 'a', 0, G_OPTION_ARG_STRING, &aitask, "select AI task to be run: [facedetect|ssd|refinedet]" },
    { NULL }
};

gint main (gint argc, gchar *argv[])
{
    GOptionContext *optctx;
    GError *error = NULL;
    App*        app         = &s_app;
    GstBus*     bus         = NULL;
    GstPad*     pad         = NULL;
    GThread*    thread      = NULL;
    gint        ret         = 0;
    gulong      hid_need    = 0;   /* handler id - need data signal       */
    gulong      hid_enough  = 0;   /* handler id - enough data signal     */
    gulong      hid_sample  = 0;   /* handler id - new sample signal      */
    gulong      pid_query   = 0;   /* probe   id - appsink query callback */
    memset (app, 0, sizeof(App));

    gst_init (&argc, &argv);
    setenv("VVAS_CORE_CFG_PATH", "/usr/share/vvas/pcie-gst-app/image_processing.cfg", 1);

    optctx = g_option_context_new ("- Application for facedetion detction on SoM board of Xilinx.");
    g_option_context_add_main_entries (optctx, entries, NULL);
    g_option_context_add_group (optctx, gst_init_get_option_group ());
    if (!g_option_context_parse (optctx, &argc, &argv, &error)) {
        g_printerr ("Error parsing options: %s\n", error->message);
        g_option_context_free (optctx);
        g_clear_error (&error);
        return -1;
    }
    g_option_context_free (optctx);


    GST_DEBUG_CATEGORY_INIT(pcie_gst_app_debug, "pcie_gst_app", 0,
            "PCIe endpoint device GStreamer application");

    app->fd = pcie_open();
    if (app->fd < 0) {
        g_printerr ("Failed to open device %d\n", app->fd);
        return PCIE_GST_APP_FAIL;
    }
    GST_DEBUG ("PCIe open success, fd = %d", app->fd);

    thread = g_thread_new ("host-app reg read thread",
                           &host_app_reg_read,
                           app);

    /* Setting up the gst elements*/
    ret = gst_set_elements(app);
    if (ret <   0) {
        g_printerr ("Failed to set the gst elements\n");
        goto PCIE_DEV_CLOSE;
    }

    /* Setting up the host parametes */
    ret = set_host_parameters(app);
    if (ret < 0) {
        g_printerr ("Failed to set the host parameters\n");
        goto GST_RESET_ELEMENTS;
    }

    /* Set Gstreamer elements properties for pipeline */
    set_property (app);

    /* Create GStreamer pipeline */
    ret = create_pipeline(app);
    if (ret < 0) {
        g_printerr("Failed to create pipeline\n");
        goto GST_RESET_ELEMENTS;
    }

    /* Create a mainloop  */
    app->loop = g_main_loop_new (NULL, TRUE);

    bus = gst_pipeline_get_bus (GST_PIPELINE (app->pipeline));

    if (app->h_param.usecase > VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS ) { 
        /* Register required callbacks */
        hid_need   = g_signal_connect(app->pciesrc,             \
                "need-data",                                    \
                G_CALLBACK(start_feed),                         \
                app);
        hid_enough = g_signal_connect(app->pciesrc,             \
                "enough-data",                                  \
                G_CALLBACK(stop_feed),                          \
                app);
    }

    if (app->h_param.usecase < VGST_USECASE_TYPE_APPSRC_TO_KMSSINK){
    hid_sample = g_signal_connect(app->pciesink,                \
            "new-sample",                                       \
            G_CALLBACK(new_sample_cb),                          \
            app);

    pad = gst_element_get_static_pad (app->pciesink, "sink");
    pid_query = gst_pad_add_probe (pad,                         \
            GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM,                \
            appsink_query_cb,                                   \
            NULL,                                               \
            NULL);
	}
	
    /* Add watch for messages */
    gst_bus_add_watch (bus, (GstBusFunc) bus_message, app);

    /* Set export fd size */
    app->export_fd_size = get_export_fd_size(app->yuv_frame_size);
    GST_DEBUG ("export fd size = %lu", app->export_fd_size);

    if (app->h_param.usecase > VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS) {
        app->dma_export.fd = 0;
        app->dma_export.size = app->export_fd_size;
        /* Driver will initalize bufferpool and store FDs, which can be
           export to user via dma-map/unmap IOCTLs */
        ret = pcie_dma_export(app->fd, &app->dma_export);
        if (ret < 0) {
            g_printerr ("Failed to initialize bufferpool");
            goto DESTROY_PIPELINE;
        }
    }

    /* move pipeline to playing state  */
    gst_element_set_state (app->pipeline, GST_STATE_PLAYING);

    g_main_loop_run (app->loop);
    GST_DEBUG ("Exiting the app...");

    gst_element_set_state (app->pipeline, GST_STATE_NULL);

    if (app->h_param.usecase > VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS) {
        /* release bufferpool memory */
        ret = pcie_dma_export_release(app->fd, &app->dma_export);
        if (ret < 0)
            g_printerr ("Failed to release bufferpool");
    }

    /* set read transfer done and write transfer done, so that
       host application can gracefuly restarts */
    read_write_transfer_done(app);

DESTROY_PIPELINE:

    /* Remove and unref pad probe */
    if (app->h_param.usecase < VGST_USECASE_TYPE_APPSRC_TO_KMSSINK){
       gst_pad_remove_probe (pad, pid_query);
       gst_object_unref (pad);
       pad = NULL;
       GST_DEBUG ("Removed pad probe");
    }
    /* Unregister singal handler */
    if (app->h_param.usecase > VGST_USECASE_TYPE_MIPISRC_TO_HOST_BYPASS) {
        g_signal_handler_disconnect (app->pciesrc,  hid_need);
        g_signal_handler_disconnect (app->pciesrc,  hid_enough);
    }
    if (app->h_param.usecase < VGST_USECASE_TYPE_APPSRC_TO_KMSSINK) {
    g_signal_handler_disconnect (app->pciesink, hid_sample);
    GST_DEBUG ("Disconnected registered signal callbacks");
	}
    /* Remove and unref bus watch */
    gst_bus_remove_watch (bus);
    gst_object_unref (bus);
    bus = NULL;
    GST_DEBUG ("Removed bus watch handler");

    /* Unref loop */
    if(app->loop)
        g_main_loop_unref(app->loop);
    app->loop = NULL;
    GST_DEBUG ("Unref main loop");

    /* Destroy GStreamer pipeline */
    GST_DEBUG("Destroying pipeline");
    destroy_pipeline (app);

GST_RESET_ELEMENTS:

    /* Release GStreamer elements */
    GST_DEBUG("Releasing GStreamer elements");
    gst_reset_elements(app);

PCIE_DEV_CLOSE:

    /* join thread */
    app->exit_thread = TRUE;
    g_thread_join (thread);

    pcie_close(app->fd);
    GST_DEBUG ("Closed PCIe FD");

    /* free debug category */
    gst_debug_category_free(pcie_gst_app_debug);
    GST_DEBUG ("freed gst debug device category");

    /* Gstreamer deinit */
    gst_deinit();
    GST_DEBUG ("GStreamer deinitialized");

    return 0;
}
