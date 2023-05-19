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

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/videodev2.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "helper.h"
#include "drm_helper.h"
#include <drm/drm_fourcc.h>
#include "common.h"

#define container_of(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) \
    *__mptr = (ptr);\
    (type *)( (char *)__mptr - offsetof(type,member) );})


/*Parse string format and extract width,height,left,top params */
static inline int parse_rect(char *s, struct v4l2_rect *r)
{
	 return sscanf(s, "%d,%d@%d,%d", &r->width, &r->height,
		 &r->left, &r->top) != 4;
}

static const char __attribute__((__unused__)) *plane_type2str(plane_type type)
{
	switch (type) {
	case PLANE_CURSOR:
		return "cursor";
	case PLANE_OVERLAY:
		return "overlay";
	case PLANE_PRIMARY:
		return "primary";
	default:
		return "invalid/unknown";
	}
}

void drm_buffer_destroy(int fd, struct drm_buffer *b)
{
	struct drm_mode_destroy_dumb destroy_dumb_obj;

	/* unmap drm buffer */
	munmap(b->drm_buff, b->dumb_buff_length);
	close(b->dbuf_fd);

	/* free-up framebuffer */
	drmModeRmFB(fd, b->fb_handle);

	memset(&destroy_dumb_obj, 0, sizeof(destroy_dumb_obj));
	destroy_dumb_obj.handle = b->bo_handle;
	/* destroy dumb buffer */
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_dumb_obj);
}

/*
 * Create dumb buffers and framebuffer for scanout.
 * Requests the DRM subsystem to prepare the buffer for memory-mapping
 */
int drm_buffer_create(struct drm_device *dev, struct drm_buffer *b,
		size_t drm_width, size_t drm_height, size_t drm_stride,
		uint32_t fourcc)
{
	struct drm_mode_create_dumb gem;
	struct drm_mode_map_dumb mreq;
	struct drm_mode_destroy_dumb gem_destroy;
	int ret;

	vlib_dbg("%s :: width:%zu height:%zu stride:%zu\n", __func__,
		 drm_width, drm_height, drm_stride);

	memset(&gem, 0, sizeof(gem));
	gem.width = drm_width;
	gem.height = drm_height;
	gem.bpp = drm_stride / drm_width * 8;

	/*
	 * Creates a gem object.
	 * The kernel will return a 32bit handle that can be used to
	 * manage the buffer with the DRM API
	 */
	ret = ioctl(dev->fd, DRM_IOCTL_MODE_CREATE_DUMB, &gem);
	if (ret) {
		VLIB_REPORT_ERR("CREATE_DUMB failed: %s", strerror(errno));
		return ret;
	}

	b->bo_handle = gem.handle;
	b->dumb_buff_length = gem.size;
	struct drm_prime_handle prime;
	memset(&prime, 0, sizeof(prime));
	prime.handle = b->bo_handle;
	/* Export gem object  to a FD */
	ret = ioctl(dev->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
	if (ret) {
		VLIB_REPORT_ERR("PRIME_HANDLE_TO_FD failed: %s",
				strerror(errno));
		goto fail_gem;
	}

	b->dbuf_fd = prime.fd;

	uint32_t offsets[4] = { 0 };
	uint32_t pitches[4] = {drm_stride};
	uint32_t bo_handles[4] = {b->bo_handle};

	vlib_dbg("drmModeAddFB2 (args):: %zu %zu %.4s\n", drm_width, drm_height,
		 (const char *)&fourcc);
	/* request the creation of frame buffers */
	ret = drmModeAddFB2(dev->fd, drm_width, drm_height, fourcc, bo_handles,
		pitches, offsets, &b->fb_handle, 0);
	if (ret) {
		VLIB_REPORT_ERR("drmModeAddFB2 failed: %s", strerror(errno));
		goto fail_prime;
	}

	/* prepare buffer for memory mapping */
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = b->bo_handle;
	ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (ret) {
		VLIB_REPORT_ERR("cannot map dumb buffer: %s", strerror(errno));
		goto fail_map;
	}

	/* perform actual memory mapping */
	b->drm_buff = mmap(0, gem.size, PROT_READ | PROT_WRITE, MAP_SHARED,
			  dev->fd, mreq.offset);
	if (b->drm_buff == MAP_FAILED) {
		VLIB_REPORT_ERR("cannot mmap dumb buffer: %s", strerror(errno));
		ret = -errno;
		goto fail_map;
	}

	return VLIB_SUCCESS;

	munmap(b->drm_buff, b->dumb_buff_length);
fail_map:
	drmModeRmFB(dev->fd, b->fb_handle);
fail_prime:
	close(b->dbuf_fd);

fail_gem:
	memset(&gem_destroy, 0, sizeof(gem_destroy));
	gem_destroy.handle = b->bo_handle;
	if (ioctl(dev->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &gem_destroy)) {
		VLIB_REPORT_ERR("DESTROY_DUMB failed: %s", strerror(errno));
		return VLIB_ERROR_INTERNAL;
	}

	return ret;
}

/* Find available CRTC and connector for scanout */
static int drm_find_crtc(struct drm_device *dev)
{
	int ret = -1;

	drmModeRes *res = drmModeGetResources(dev->fd);
	if (!res) {
		VLIB_REPORT_ERR("drmModeGetResources failed: %s",
				strerror(errno));
		return VLIB_ERROR_INTERNAL;
	}

	if (res->count_crtcs <= 0) {
		VLIB_REPORT_ERR("drm: no crts");
		goto done;
	}

	/* Assume first crtc id is ok */
	dev->crtc_index = 0 ;
	dev->crtc_id = res->crtcs[0] ;

	if (res->count_connectors <= 0) {
		VLIB_REPORT_ERR("drm: no connectors");
		goto done;
	}

	/* Assume first connector is ok */
	dev->con_id = res->connectors[0];

	dev->connector = drmModeGetConnector(dev->fd, dev->con_id);
	if (!dev->connector) {
		VLIB_REPORT_ERR("drmModeGetConnector failed: %s",
				strerror(errno));
		goto done;
	}

	ret = VLIB_SUCCESS;

done:
	drmModeFreeResources(res);
	return ret;
}

/**
 * drm_try_mode - Check if a mode with matching resolution is valid
 * @dev: Pointer to DRM data structure
 * @width: Desired mode width
 * @height: Desired mode height
 * @vrefresh: Refresh rate of found mode
 *
 * Search for a mode that supports the desired @widthx@height. If a matching
 * mode is found @vrefresh is populated with the refresh rate for that mode.
 *
 * Return: 0 on success, error code otherwise.
 */
int drm_try_mode(struct drm_device *dev, int width, int height, size_t *vrefresh)
{
	int ret;

	dev->fd = open(dev->dri_card, O_RDWR, 0);
	ASSERT2(dev->fd >= 0, "open DRM device %s failed: %s\n", dev->dri_card,
		ERRSTR);

	ret = drm_find_crtc(dev);
	ASSERT2(!ret, "Failed to find CRTC and/or connector\n");

	drmModeConnector *connector = dev->connector;
	ret = VLIB_ERROR_INVALID_PARAM;

	for (size_t j = 0; j < connector->count_modes; j++) {
		/* Iterate through all the supported modes */
		if (connector->modes[j].hdisplay == width &&
			connector->modes[j].vdisplay == height) {
			if (vrefresh) {
				*vrefresh = connector->modes[j].vrefresh;
			}
			ret = VLIB_SUCCESS;
			break;
		}
	}

	drmModeFreeConnector(dev->connector);
	drmClose(dev->fd);

	return ret;
}

static int drm_set_mode(struct drm_device *dev, const char *bgnd)
{
	size_t j;
	uint32_t fourcc = 0;
	int ret, mode_found = 0;
	drmModeCrtc *curr_crtc;

	struct video_pipeline *v_pipe = container_of(dev, struct video_pipeline,
						     drm);

	dev->saved_crtc= drmModeGetCrtc(dev->fd, dev->crtc_id);
	ASSERT2(dev->saved_crtc, "Could not get crtc %i\n", dev->crtc_id);

	curr_crtc = dev->saved_crtc;

	drmModeConnector *connector = dev->connector;
	ret = VLIB_SUCCESS;
	for (j = 0; j < connector->count_modes; j++) {
		size_t vrefresh = dev->vrefresh;
		drmModeModeInfoPtr mode = &connector->modes[j];

		/* Iterate through all the supported modes */
		if (mode->hdisplay == v_pipe->w_out &&
		    mode->vdisplay== v_pipe->h_out) {
			if (vrefresh && !(mode->vrefresh == vrefresh)) {
				continue;
			}

			mode_found = 1;
			v_pipe->vtotal = mode->vtotal;
			v_pipe->htotal = mode->htotal;
			dev->fps = mode->vrefresh;
			break;
		}
	}
	/* Assert if requested resolution is not supported by the CRTC display */
	ASSERT2(mode_found,
		"Input Resolution %dx%d not supported by the monitor!\n",
		v_pipe->w_out, v_pipe->h_out);

	drmModePlanePtr plane = drmModeGetPlane(dev->fd,
				dev->prim_plane.drm_plane->plane_id);
	ASSERT2(plane, "failed to get primary plane\n");
	ASSERT2(plane->count_formats, "plane does not support any formats\n");

	/*
	 * find best format for CRTC in the following priority: AR24, RG24, any.
	 * AR24 and RG24 are the 4/3 byte formats used by X11. If none of those
	 * is available use the first format the plane supports.
	 */
	fourcc = plane->formats[0];
	for (size_t i = 0; i < plane->count_formats; i++) {
		if (plane->formats[i] == V4L2_PIX_FMT_ABGR32) {
			fourcc = V4L2_PIX_FMT_ABGR32;
			break;
		}
		if (plane->formats[i] == DRM_FORMAT_RGB888) {
			fourcc = DRM_FORMAT_RGB888;
		}
	}

	drmModeFreePlane(plane);

	ret = drm_buffer_create(dev, &dev->crtc_buf,
				v_pipe->w_out, v_pipe->h_out,
				v_pipe->w_out * vlib_fourcc2bpp(fourcc),
				fourcc);
	ASSERT2(!ret, "failed to create CRTC buffer\n");

	/* get background data */
	if (!bgnd) {
		goto set_crtc;
	}

	FILE *fd = fopen(bgnd, "r");
	if (fd < 0) {
		VLIB_REPORT_ERR("unable to open file '%s': %s",
				bgnd, strerror(errno));
		ret = VLIB_ERROR_INVALID_PARAM;
		goto err_fopen;
	}

	size_t frame_sz = v_pipe->w_out * v_pipe->h_out * vlib_fourcc2bpp(fourcc);
	ret = fread(dev->crtc_buf.drm_buff, frame_sz, 1, fd);
	if (ret != 1) {
		VLIB_REPORT_ERR("failed to read background image: %s",
				strerror(errno));
		ret = VLIB_ERROR_FILE_IO;
		goto err_fread;
	}

	fclose(fd);

set_crtc:
	/* Set the resolution */
	ret = drmModeSetCrtc(dev->fd, curr_crtc->crtc_id,
			     dev->crtc_buf.fb_handle,
			     0, 0,
			     &dev->con_id, 1, &connector->modes[j]);
	ASSERT2(ret >= 0,
		"drmModeSetCrtc :: Failed Not able to set resolution [%dx%d] on the CRTC: %s\n",
		v_pipe->w_out, v_pipe->h_out, ERRSTR);

	return 0;

err_fread:
	fclose(fd);
err_fopen:
	drm_buffer_destroy(dev->fd, &dev->crtc_buf);

	return ret;
}

/**
 * drm_find_preferred_mode
 * @dev:	Pointer to DRM struct
 *
 * This function finds the preferred mode of the DRM device. This function must
 * only be called after drm_init()
 *
 * Return: 0 on success, errorcode otherwise.
 */

int drm_find_preferred_mode(struct drm_device *dev)
{
	ASSERT2(dev->fd >= 0, "open DRM device %s failed: %s\n", dev->dri_card,
		ERRSTR);
	ASSERT2(dev->connector, "no connector found\n");

	drmModeConnector *connector = dev->connector;

	if (!connector->count_modes) {
		VLIB_REPORT_ERR("connector supports no mode");
		return VLIB_ERROR_INTERNAL;
	}

	/* First advertised mode is preferred mode */
	dev->preferred_mode = connector->modes;

	return 0;
}

/**
 * drm_find_overlay_plane - test if plane is suitable overlay plane
 * @dev:	Pointer to DRM struct
 * @p:		Pointer to plane properties
 * @plane:	Pointer to plane to test
 *
 * Test if @plane has suitable properties to server as overlay plane for @dev.
 *
 * Return: 1 if @plane is valid overlay plane, 0 otherwise
 */
static int drm_find_overlay_plane(struct drm_device *dev, struct vlib_plane *p,
				  drmModePlanePtr plane)
{
	size_t i;

	if (dev->overlay_plane.drm_plane) {
		return 0;
	}

	if (!(plane->possible_crtcs & (1 << dev->crtc_index))) {
		return 0;
	}

	if (p->id && p->id != plane->plane_id) {
		return 0;
	}

	for (i = 0; i < plane->count_formats; ++i) {
		if (plane->formats[i] == dev->format)
			break;
	}

	if (i == plane->count_formats) {
		return 0;
	}

	return 1;
}

static plane_type drm_get_plane_type(struct drm_device *dev, unsigned int plane_id)
{
	drmModeObjectPropertiesPtr props;
	plane_type type = PLANE_NONE;
	int found = 0;

	props = drmModeObjectGetProperties(dev->fd, plane_id,
					   DRM_MODE_OBJECT_PLANE);
	ASSERT2(props, "DRM get_properties failed\n");

	for (size_t i = 0; i < props->count_props && !found; i++) {
		drmModePropertyPtr prop;
		const char *enum_name = NULL;

		prop = drmModeGetProperty(dev->fd, props->props[i]);
		ASSERT2(prop, "DRM get_property failed\n");

		if (strcmp(prop->name, "type") == 0) {
			for (size_t j = 0; j < prop->count_enums; j++) {
				if (prop->enums[j].value ==
				    props->prop_values[i]) {
					enum_name = prop->enums[j].name;
					break;
				}
			}

			if (strcmp(enum_name, "Primary") == 0)
				type = PLANE_PRIMARY;
			else if (strcmp(enum_name, "Overlay") == 0)
				type = PLANE_OVERLAY;
			else if (strcmp(enum_name, "Cursor") == 0)
				type = PLANE_CURSOR;
			else
				vlib_warn("Invalid DRM Plane type\n");

			found = 1;
		}

		drmModeFreeProperty(prop);
	}

	ASSERT2(found, "Invalid DRM Plane type\n");
	drmModeFreeObjectProperties(props);

	return type;
}

/* Find an unused plane that supports the requested format */
static int drm_find_plane(struct drm_device *dev, struct vlib_plane *p)
{
	drmModePlaneResPtr planes;
	int ret = -1;

	vlib_dbg("%s\n\n", __func__);

	planes = drmModeGetPlaneResources(dev->fd);
	if (!planes) {
		VLIB_REPORT_ERR("drmModeGetPlaneResources failed: %s",
				strerror(errno));
		return VLIB_ERROR_INTERNAL;
	}

	for (size_t i = 0; i < planes->count_planes &&
	     !(dev->prim_plane.drm_plane && dev->overlay_plane.drm_plane); ++i) {
		drmModePlanePtr plane = drmModeGetPlane(dev->fd, planes->planes[i]);
		if (!plane) {
			VLIB_REPORT_ERR("drmModeGetPlane failed: %s",
					strerror(errno));
			break;
		}

		/* Retrieve plane type - PRIMARY, OVERLAY or CURSOR */
		plane_type type = drm_get_plane_type(dev, plane->plane_id);

		vlib_dbg("plane %zu/%d:\n", i + 1, planes->count_planes);
		vlib_dbg("\tcrtc id: %d\n", plane->crtc_id);
		vlib_dbg("\tplane id: %d\n", plane->plane_id);
		vlib_dbg("\tplane format: %.4s\n", (const char *)&plane->formats[0]);
		vlib_dbg("\tplane type: %s\n\n", plane_type2str(type));

		if (type == PLANE_PRIMARY) {
			dev->prim_plane.drm_plane = plane;

			if (dev->overlay_plane.drm_plane) {
				break;
			}
		}

		ret = drm_find_overlay_plane(dev, p, plane);
		if (ret) {
			dev->overlay_plane.drm_plane = plane;
			ret = 0;
		} else if (!dev->prim_plane.drm_plane ||
			    dev->prim_plane.drm_plane != plane) {
			drmModeFreePlane(plane);
		}
	}

	drmModeFreePlaneResources(planes);
	return ret;
}

/* Initialize DRM module query CRTC/Plane configuration*/
void drm_init(struct drm_device *dev, struct vlib_plane *plane)
{
	int ret;

	dev->fd = open(dev->dri_card, O_RDWR, 0);
	ASSERT2(dev->fd >= 0, "open DRM device %s failed: %s\n", dev->dri_card,
		ERRSTR);

	drmSetVersion sv;
	memset(&sv, 0, sizeof(sv));
	sv.drm_di_major = 1;
	sv.drm_di_minor = 4;
	sv.drm_dd_major = -1;
	sv.drm_dd_minor = -1;
	ret = drmSetInterfaceVersion(dev->fd, &sv);
	ASSERT2(!ret, "failed to set DRM interface version\n");

	ret = drm_find_crtc(dev);
	ASSERT2(!ret, "failed to find CRTC and/or connector\n");

	/* enable universal plane */
	ret = drmSetClientCap(dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	ASSERT2(!ret, "universal plane not supported\n");

	ret = drm_find_plane(dev, plane);
	ASSERT2(!ret, "failed to find compatible plane\n");
}

/* Allocate frame-buffer for display, creates user-space mapping and set CRTC mode*/
void drm_post_init(struct drm_device *dev, const char *bgnd)
{
	int ret;
	struct video_pipeline *v_pipe;

	v_pipe = container_of(dev, struct video_pipeline, drm);

	for (size_t i = 0; i < dev->buffer_cnt; ++i) {
		ret = drm_buffer_create(dev, &dev->d_buff[i],
					dev->overlay_plane.vlib_plane.width,
					dev->overlay_plane.vlib_plane.height,
					v_pipe->stride_out, dev->format);
		dev->d_buff[i].index=i;
		ASSERT2(!ret, "failed to create buffer%zu\n", i);
	}
	vlib_dbg("buffers ready\n");

	/* Startup DRM settings */
	if (v_pipe->app_state == MODE_CHANGE || v_pipe->app_state == MODE_INIT)
		drm_set_mode(dev, bgnd);
}

/* Un-initialize drm module , freeup allocated resources */
void drm_uninit (struct drm_device *dev)
{
	/* delete dumb buffers */
	for (size_t i = 0; i < dev->buffer_cnt; i++) {
		drm_buffer_destroy(dev->fd, &dev->d_buff[i]);
	}

	drm_buffer_destroy(dev->fd, &dev->crtc_buf);

	drmModeFreePlane(dev->prim_plane.drm_plane);
	drmModeFreePlane(dev->overlay_plane.drm_plane);

	/* restore saved CRTC configuration */
	drmModeSetCrtc(dev->fd, dev->saved_crtc->crtc_id,
				dev->saved_crtc->buffer_id,
				dev->saved_crtc->x,
				dev->saved_crtc->y,
				&dev->con_id,
				1,
				&dev->saved_crtc->mode);
	drmModeFreeCrtc(dev->saved_crtc);
	drmModeFreeConnector(dev->connector);
	drmDropMaster(dev->fd);
	close(dev->fd);
	free(dev->d_buff);
}

/* Configures plane with buffer index to be selected for next scanout */
int drm_set_plane(struct drm_device *dev, int index)
{
	/*
	 * Configure plane, the crtc then blends the content from the
	 * plane over the CRTC framebuffer buffer during scanout
	 */
	return drmModeSetPlane(dev->fd, dev->overlay_plane.drm_plane->plane_id,
				dev->crtc_id, dev->d_buff[index].fb_handle, 0,
				dev->overlay_plane.vlib_plane.xoffs, /* crtx_x */
				dev->overlay_plane.vlib_plane.yoffs, /* crtc_y */
				dev->overlay_plane.vlib_plane.width, /* crtc_w */
				dev->overlay_plane.vlib_plane.height, /* crtc_h */
				0, 0, /* src_x, src_y */
				dev->overlay_plane.vlib_plane.width << 16, /* src_w */
				dev->overlay_plane.vlib_plane.height << 16); /* src_h */
}

int drm_wait_vblank(struct drm_device *dev, void *d_ptr)
{
	int ret;
	drmVBlank vblank;
	vblank.request.type = DRM_VBLANK_EVENT | DRM_VBLANK_RELATIVE;
	vblank.request.sequence = 1;
	vblank.request.signal = (unsigned long)d_ptr;
	ret = drmWaitVBlank(dev->fd, &vblank);
	ASSERT2(!ret, "drmWaitVBlank failed: %s\n", ERRSTR);
	return VLIB_SUCCESS;
}

/* Set DRM plane property for input property name and value */
int drm_set_plane_prop(struct drm_device *dev, unsigned int plane_id, const char *prop_name, int prop_val)
{
	drmModeObjectPropertiesPtr props;
	int ret = -1;

	props = drmModeObjectGetProperties(dev->fd, plane_id, DRM_MODE_OBJECT_PLANE);
	if (!props) {
		return ret;
	}

	for (size_t i = 0; i < props->count_props; i++) {
		drmModePropertyPtr prop = drmModeGetProperty(dev->fd, props->props[i]);

		if (!strcmp(prop->name, prop_name)) {
			ret = drmModeObjectSetProperty(dev->fd, plane_id,
						       DRM_MODE_OBJECT_PLANE,
						       prop->prop_id,
						       prop_val);
			drmModeFreeProperty(prop);
			break;
		}
		drmModeFreeProperty(prop);
	}
	drmModeFreeObjectProperties(props);

	return ret;
}

int drm_set_plane_state(struct drm_device *dev, unsigned int plane_id, int enable)
{
	int fb_id = 0, flags = 0;
	drmModePlanePtr plane = drmModeGetPlane(dev->fd, plane_id);

	/* If plane is to be enabled restore original frame-buffer id
	    to disable it set it to NULL*/
	if (enable)
		fb_id = plane->fb_id;

	vlib_dbg("%s :: %d %d %d %d %d %d\n", __func__,
		 plane->plane_id,dev->crtc_id,plane->crtc_x, plane->crtc_y,
		 plane->x, plane->y);
	/* note src coords (last 4 args) are in Q16 format */
	drmModeSetPlane(dev->fd, plane->plane_id, dev->crtc_id, fb_id, flags,
			dev->overlay_plane.vlib_plane.xoffs, /* crtx_x */
			dev->overlay_plane.vlib_plane.yoffs, /* crtc_y */
			dev->overlay_plane.vlib_plane.width, /* crtc_w */
			dev->overlay_plane.vlib_plane.height, /* crtc_h */
			0, 0, /* src_x, src_y */
			dev->overlay_plane.vlib_plane.width << 16, /* src_w */
			dev->overlay_plane.vlib_plane.height << 16); /* src_h */

	drmModeFreePlane(plane);

	return VLIB_SUCCESS;
}

/* Set primary plane offset (x,y) */
/* TODO: Make it generic for all planes */
int drm_set_prim_plane_pos(struct drm_device *dev, int x, int y)
{
	int flags = 0;
	drmModePlanePtr prim_plane = dev->prim_plane.drm_plane;

	struct video_pipeline *v_pipe;
	v_pipe = container_of(dev, struct video_pipeline, drm);

	prim_plane->crtc_x = x;
	prim_plane->crtc_y = y;

	vlib_dbg("%s :: %d %d %d %d %d %d %d\n", __func__,
		 prim_plane->plane_id,dev->crtc_id, prim_plane->fb_id,
		 prim_plane->crtc_x, prim_plane->crtc_y, prim_plane->x,
		 prim_plane->y);

	/* note src coords (last 4 args) are in Q16 format */
	drmModeSetPlane(dev->fd, prim_plane->plane_id, dev->crtc_id,
			prim_plane->fb_id, flags, prim_plane->crtc_x,
			prim_plane->crtc_y, v_pipe->w_out,
			v_pipe->h_out - prim_plane->crtc_y, 0, 0, v_pipe->w_out << 16,
			(v_pipe->h_out - prim_plane->crtc_y) << 16);

	return VLIB_SUCCESS;
}
