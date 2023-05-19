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

#include <pcie_sink.h>

GST_DEBUG_CATEGORY_EXTERN (pcie_gst_app_debug);
#define GST_CAT_DEFAULT pcie_gst_app_debug

GstFlowReturn new_sample_cb (GstElement* elt, App* app)
{
    GstSample *sample = NULL;
    GstBuffer *buffer = NULL;
    GstMemory *mem    = NULL;
    gint      ret     = 0;

    app->appsink_framecnt++;

    /* get the sample from appsink */
    sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));

    /* get buffer from sample */
    buffer = gst_sample_get_buffer (sample);
    if (!buffer) {
        GST_ERROR ("Appsink: received NULL buffer");
        return GST_FLOW_EOS;
    }

    /* get memory from buffer */
    mem = gst_buffer_peek_memory (buffer, 0);
    if(!mem) {
        GST_ERROR ("Appsink: recieved NULL memory");
        return GST_FLOW_EOS;
    }

    /* is memory dmabuf type? */
    if (gst_is_dmabuf_memory (mem)) {
        GST_DEBUG ("Appsink: pulled dmabuf element, frame-count -> %lu",
                  app->appsink_framecnt);
    } else {
        GST_ERROR ("Appsink: pulled non-dmabuf memory");
        return GST_FLOW_EOS;
    }

    /* get fd from memory */
    app->dma_import.dbuf_fd = gst_dmabuf_memory_get_fd (mem);
    GST_DEBUG ("Appsink: received fd - %d", app->dma_import.dbuf_fd);

    /* request driver to import the fd */
    ret = pcie_dma_import(app->fd,&app->dma_import);
    if (ret < 0 ) {
        GST_ERROR ("Failed to get import fd");
    }
    GST_DEBUG ("Appsink: dma import successful");

    /* initiate dma write operation */
    ret = pcie_write(app->fd, app->yuv_frame_size, 0, NULL);
    if (ret < 0) {
        GST_ERROR ("pcie_write failed, err - %d", ret);
    }
    GST_DEBUG ("Appsink: pcie write successful");

    /* all done, release the fd */
    ret = pcie_dma_import_release(app->fd,&app->dma_import);
    if (ret < 0 ) {
        GST_ERROR ("Failed to release import dma buf fd - %d",
                app->dma_import.dbuf_fd);
    }
    GST_DEBUG ("Appsink: dma import release successful");

    gst_sample_unref (sample);
    GST_DEBUG ("Appsink: processed frame-count -> %lu", app->appsink_framecnt);

    return GST_FLOW_OK;
}
