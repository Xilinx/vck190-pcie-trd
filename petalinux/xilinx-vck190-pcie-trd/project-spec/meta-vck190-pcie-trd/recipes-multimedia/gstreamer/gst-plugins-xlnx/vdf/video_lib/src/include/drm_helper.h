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

#ifndef DRM_HELPER_H
#define DRM_HELPER_H

#include <sys/types.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <common.h>

typedef enum {
	PLANE_OVERLAY,
	PLANE_PRIMARY,
	PLANE_CURSOR,
	PLANE_NONE
} plane_type;

struct drm_buffer {
	unsigned int index;
	unsigned int bo_handle;
	unsigned int fb_handle;
	int dbuf_fd;			/* DRM kernel buffer FD */
	unsigned char *drm_buff;
	unsigned int dumb_buff_length;
};

struct vlib_drm_plane {
	struct vlib_plane vlib_plane;
	drmModePlanePtr drm_plane;
};

struct drm_device {
	char dri_card[32];
	int fd;
	int crtc_index;
	unsigned int crtc_id;
	unsigned int con_id;
	struct drm_buffer crtc_buf;
	struct vlib_drm_plane prim_plane;
	struct vlib_drm_plane overlay_plane;
	drmModeConnector *connector;
	drmModeModeInfo *preferred_mode;
	drmModeCrtc *saved_crtc;
	unsigned int format;
	struct drm_buffer *d_buff;
	size_t buffer_cnt;
	unsigned int fps;
	size_t vrefresh;
};

#include <video_int.h>

void drm_init(struct drm_device *dev, struct vlib_plane *plane);
void drm_post_init(struct drm_device *dev, const char *bgnd);
int drm_buffer_create(struct drm_device *dev, struct drm_buffer *b,
		size_t drm_width, size_t drm_height, size_t drm_stride,
		uint32_t fourcc);
void drm_buffer_destroy(int fd, struct drm_buffer *b);
/* Configures plane with buffer index to be selected for next scanout */
int drm_set_plane(struct drm_device *, int index);
/*Request a Vblank event*/
int drm_wait_vblank(struct drm_device *, void *d_ptr);
/* Set DRM plane property for input property name and value */
int drm_set_plane_prop(struct drm_device *dev, unsigned int plane_id, const char *prop_name, int prop_val);
/* Un-initialize drm module , freeup allocated resources */
void drm_uninit (struct drm_device *dev) ;
/* Enable/disable plane */
int drm_set_plane_state(struct drm_device *dev, unsigned int plane_id, int enable);
/* Set primary plane offset (x,y) */
int drm_set_prim_plane_pos(struct drm_device *dev, int x, int y);
/* Find DRM preferred mode */
int drm_find_preferred_mode(struct drm_device *dev);
/* Validate DRM resolution */
int drm_try_mode(struct drm_device *dev, int width, int height, size_t *vrefresh);
#endif /* DRM_HELPER_H */
