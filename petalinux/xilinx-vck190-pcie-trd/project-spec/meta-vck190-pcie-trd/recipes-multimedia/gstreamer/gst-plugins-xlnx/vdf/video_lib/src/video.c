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

#include <glib.h>
#include <glob.h>
#include <poll.h>
#include <stdio.h>
#include <drm/drm_fourcc.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "platform.h"
#include "common.h"
#include "helper.h"
#include "gpio_utils.h"
#include "video_int.h"
#include "mediactl_helper.h"

/* Maximum number of bytes in a log line */
#define VLIB_LOG_SIZE 256

/* number of frame buffers */
#define BUFFER_CNT_MIN     6
#define BUFFER_CNT_DEFAULT 6

/* global variables */
char vlib_errstr[VLIB_ERRSTR_SIZE];

static struct video_pipeline *video_setup;
static GMutex lock;

int vlib_platform_setup(struct vlib_config_data *cfg)
{
	int ret = 0;

	switch (cfg->display_id) {
	case DRI_CARD_DP:
	case DRI_CARD_HDMI:
		ret = vlib_platform_set_qos(cfg->display_id);
		if (ret) {
			return ret;
		}
		break;
	default:
		VLIB_REPORT_ERR("No valid DRM device found");
		return VLIB_ERROR_INVALID_PARAM;
		break;
	}

	return ret;
}

static int vlib_drm_id2card(struct drm_device *dev, unsigned int dri_card_id)
{
	int ret;
	glob_t pglob;
	const char *drm_str;
	unsigned int dri_card;

	switch (dri_card_id) {
	case DRI_CARD_DP:
		drm_str = "DP";
		ret = glob("/sys/class/drm/card*-DP-1", 0, NULL, &pglob);
		break;
	case DRI_CARD_HDMI:
		drm_str = "HDMI";
		ret = glob("/sys/class/drm/card*-HDMI-A-1", 0, NULL, &pglob);
		break;
	default:
		VLIB_REPORT_ERR("No valid DRM device found");
		ret = VLIB_ERROR_INVALID_PARAM;
		goto error;
	}

	if (ret || pglob.gl_pathc != 1) {
		VLIB_REPORT_ERR("No %s DRM device found", drm_str);
		ret = VLIB_ERROR_OTHER;
		goto error;
	}

	sscanf(pglob.gl_pathv[0], "/sys/class/drm/card%u-*", &dri_card);
	snprintf(dev->dri_card, sizeof(dev->dri_card), "/dev/dri/card%u",
		 dri_card);
	vlib_info("%s DRM device found at %s\n", drm_str, dev->dri_card);

error:
	globfree(&pglob);

	return ret;
}

/**
 * vlib_drm_try_mode - Check if a mode with matching resolution is valid
 * @display_id: Display ID
 * @width: Desired mode width
 * @height: Desired mode height
 * @vrefresh: Refresh rate of found mode
 *
 * Search for a mode that supports the desired @widthx@height. If a matching
 * mode is found @vrefresh is populated with the refresh rate for that mode.
 *
 * Return: 0 on success, error code otherwise.
 */
int vlib_drm_try_mode(unsigned int display_id, int width, int height,
		      size_t *vrefresh)
{
	int ret;
	size_t vr;
	struct drm_device drm_dev;

	ret = vlib_drm_id2card(&drm_dev, display_id);
	if (ret) {
		return ret;
	}

	ret = drm_try_mode(&drm_dev, width, height, &vr);
	if (vrefresh) {
		*vrefresh = vr;
	}

	return ret;
}

static int vlib_drm_init(struct vlib_config_data *cfg)
{
	int ret;
	size_t bpp;
	struct drm_device *drm_dev = &video_setup->drm;

	ret = vlib_drm_id2card(drm_dev, cfg->display_id);
	if (ret) {
		return ret;
	}

	drm_dev->overlay_plane.vlib_plane = cfg->plane;
	drm_dev->format = video_setup->out_fourcc;
	drm_dev->vrefresh = cfg->vrefresh;
	drm_dev->buffer_cnt = cfg->buffer_cnt;

	drm_dev->d_buff = calloc(drm_dev->buffer_cnt, sizeof(*drm_dev->d_buff));
	ASSERT2(drm_dev->d_buff, "failed to allocate DRM buffer structs\n");

	bpp = vlib_fourcc2bpp(drm_dev->format);
	if (!bpp) {
		VLIB_REPORT_ERR("unsupported pixel format '%s'",
				(const char *)&drm_dev->format);
		return VLIB_ERROR_INVALID_PARAM;
	}

	drm_init(drm_dev, &cfg->plane);

	/* Set display resolution */
	if (!cfg->height_out) {
		/* set preferred mode */
		int ret = drm_find_preferred_mode(drm_dev);
		if (ret)
			return ret;

		video_setup->h_out = drm_dev->preferred_mode->vdisplay;
		video_setup->w_out = drm_dev->preferred_mode->hdisplay;

		if (!video_setup->h) {
			video_setup->h = video_setup->h_out;
			video_setup->w = video_setup->w_out;
			video_setup->stride = video_setup->w * bpp;
		}
	} else {
		video_setup->h_out = cfg->height_out;
		video_setup->w_out = cfg->width_out;
	}

	/* if not specified on the command line make the plane fill the whole screen */
	if (!cfg->plane.width) {
		drm_dev->overlay_plane.vlib_plane.width = video_setup->w_out;
		drm_dev->overlay_plane.vlib_plane.height = video_setup->h_out;
	}

	video_setup->stride_out = drm_dev->overlay_plane.vlib_plane.width * bpp;

	drm_post_init(drm_dev, cfg->drm_background);

	if (!(cfg->flags & VLIB_CFG_FLAG_MULTI_INSTANCE)) {
		/* Move video layer to the back and disable global alpha */
		if (drm_set_plane_prop(drm_dev,
				       video_setup->drm.overlay_plane.drm_plane->plane_id,
				       "zpos", 0)) {
			vlib_warn("failed to set zpos\n");
		}

		if (drm_set_plane_prop(drm_dev,
				       video_setup->drm.prim_plane.drm_plane->plane_id,
				       "zpos", 1) ) {
			vlib_warn("failed to set zpos\n");
		}

		if (drm_set_plane_prop(drm_dev,
				       video_setup->drm.prim_plane.drm_plane->plane_id,
				       "global alpha enable", 0) ) {
			vlib_warn("failed to set 'global alpha'\n");
		}
	}

	if (cfg->display_id == DRI_CARD_DP) {
		/*
		 * New DRM driver sets global alpha ON by default for DP-Tx
		 * Disable it here
		 */
		if (drm_set_plane_prop(drm_dev,
				       video_setup->drm.prim_plane.drm_plane->plane_id,
				       "g_alpha_en", 0) ) {
			vlib_warn("failed to disable 'global alpha'\n");
		}
	}

	vlib_dbg("vlib :: DRM Init done ..\n");

	return VLIB_SUCCESS;
}

int vlib_get_active_height(void)
{
	return video_setup->h_out;
}

int vlib_get_active_width(void)
{
	return video_setup->w_out;
}

unsigned int vlib_get_fps(void)
{
	return video_setup->fps.numerator;
}

int vlib_drm_set_layer0_state(int enable_state)
{
	/* Map primary-plane cordinates into CRTC using drmModeSetPlane */
	drm_set_plane_state(&video_setup->drm,
			    video_setup->drm.prim_plane.drm_plane->plane_id,
			    enable_state);
	return VLIB_SUCCESS;
}

int vlib_drm_set_layer0_transparency(int transparency)
{
	/* Set Layer Alpha for graphics layer */
	drm_set_plane_prop(&video_setup->drm,
			   video_setup->drm.prim_plane.drm_plane->plane_id,
			DRM_ALPHA_PROP, (DRM_MAX_ALPHA-transparency));

	return VLIB_SUCCESS;
}

int vlib_drm_set_layer0_position(int x, int y)
{
	drm_set_prim_plane_pos(&video_setup->drm, x, y);
	return VLIB_SUCCESS;
}

/** This function returns a constant NULL-terminated string with the ASCII name of a vlib
 *  error. The caller must not free() the returned string.
 *
 *  \param error_code The \ref vlib_error to return the name of.
 *  \returns The error name, or the string **UNKNOWN** if the value of
 *  error_code is not a known error.
 */
const char *vlib_error_name(vlib_error error_code)
{
	switch (error_code) {
	case VLIB_ERROR_INTERNAL:
		return "VLIB Internal Error";
	case VLIB_ERROR_CAPTURE:
		return "VLIB Capture Error";
	case VLIB_ERROR_INVALID_PARAM:
		return "VLIB Invalid Parameter Error";
	case VLIB_ERROR_FILE_IO:
		return "VLIB File I/O Error";
	case VLIB_ERROR_NOT_SUPPORTED:
		return "VLIB Not Supported Error";
	case VLIB_ERROR_OTHER:
		return "VLIB Other Error";
	case VLIB_SUCCESS:
		return "VLIB Success";
	default:
		return "VLIB Unknown Error";
	}
}

/** This function returns a string with a short description of the given error code.
 *  This description is intended for displaying to the end user.
 *
 *  The messages always start with a capital letter and end without any dot.
 *  The caller must not free() the returned string.
 *
 *  \returns a short description of the error code in UTF-8 encoding
 */
char *vlib_strerror(void)
{
	return vlib_errstr;
}

/* This function returns a string with a log-information w.r.t to the input log-level */
static void vlib_log_str(vlib_log_level level, const char *str)
{
	fputs(str, stderr);
	UNUSED(level);
}

void vlib_log_v(vlib_log_level level, const char *format, va_list args)
{
	const char *prefix = "";
	char buf[VLIB_LOG_SIZE];
	int header_len, text_len;

	switch (level) {
	case VLIB_LOG_LEVEL_INFO:
		prefix = "[vlib info] ";
		break;
	case VLIB_LOG_LEVEL_WARNING:
		prefix = "[vlib warning] ";
		break;
	case VLIB_LOG_LEVEL_ERROR:
		prefix = "[vlib error] ";
		break;
	case VLIB_LOG_LEVEL_DEBUG:
		prefix = "[vlib debug] ";
		break;
	case VLIB_LOG_LEVEL_EVENT:
		prefix = "[vlib event] ";
		break;
	case VLIB_LOG_LEVEL_NONE:
	default:
		return;
	}

	header_len = snprintf(buf, sizeof(buf), "%s", prefix);
	if (header_len < 0 || header_len >= (int)sizeof(buf)) {
		/* Somehow snprintf failed to write to the buffer,
		 * remove the header so something useful is output. */
		header_len = 0;
	}
	/* Make sure buffer is NULL terminated */
	buf[header_len] = '\0';

	text_len = vsnprintf(buf + header_len, sizeof(buf) - header_len, format, args);
	if (text_len < 0 || text_len + header_len >= (int)sizeof(buf)) {
		/* Truncated log output. On some platforms a -1 return value means
		 * that the output was truncated. */
		text_len = sizeof(buf) - header_len;
	}

	if (header_len + text_len >= sizeof(buf)) {
		/* Need to truncate the text slightly to fit on the terminator. */
		text_len -= (header_len + text_len) - sizeof(buf);
	}

	vlib_log_str(level, buf);
}

void vlib_log(vlib_log_level level, const char *format, ...)
{
	va_list args;

	va_start (args, format);
	vlib_log_v(level, format, args);
	va_end (args);
}

/**
 * vlib_fourcc2bpp - Get bytes per pixel
 * @fourcc: Fourcc pixel format code
 *
 * Return: Number of bytes per pixel for @fourcc or 0.
 */
size_t vlib_fourcc2bpp(uint32_t fourcc)
{
	size_t bpp;

	/* look up bits per pixel */
	switch (fourcc) {
	case V4L2_PIX_FMT_RGB332:
	case V4L2_PIX_FMT_HI240:
	case V4L2_PIX_FMT_HM12:
	case DRM_FORMAT_RGB332:
	case DRM_FORMAT_BGR233:
		bpp = 8;
		break;
	case V4L2_PIX_FMT_YVU410:
	case V4L2_PIX_FMT_YUV410:
		bpp = 9;
		break;
	case V4L2_PIX_FMT_YVU420:
	case V4L2_PIX_FMT_YUV420:
	case V4L2_PIX_FMT_M420:
	case V4L2_PIX_FMT_Y41P:
		bpp = 12;
		break;
	case V4L2_PIX_FMT_RGB444:
	case V4L2_PIX_FMT_ARGB444:
	case V4L2_PIX_FMT_XRGB444:
	case V4L2_PIX_FMT_RGB555:
	case V4L2_PIX_FMT_ARGB555:
	case V4L2_PIX_FMT_XRGB555:
	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_RGB555X:
	case V4L2_PIX_FMT_ARGB555X:
	case V4L2_PIX_FMT_XRGB555X:
	case V4L2_PIX_FMT_RGB565X:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_YYUV:
	case V4L2_PIX_FMT_YVYU:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_VYUY:
	case V4L2_PIX_FMT_YUV422P:
	case V4L2_PIX_FMT_YUV411P:
	case V4L2_PIX_FMT_YUV444:
	case V4L2_PIX_FMT_YUV555:
	case V4L2_PIX_FMT_YUV565:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
		bpp = 16;
		break;
	case V4L2_PIX_FMT_BGR666:
		bpp = 18;
		break;
	case V4L2_PIX_FMT_BGR24:
	case V4L2_PIX_FMT_RGB24:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_BGR888:
		bpp = 24;
		break;
	case V4L2_PIX_FMT_BGR32:
	case V4L2_PIX_FMT_ABGR32:
	case V4L2_PIX_FMT_XBGR32:
	case V4L2_PIX_FMT_RGB32:
	case V4L2_PIX_FMT_ARGB32:
	case V4L2_PIX_FMT_XRGB32:
	case V4L2_PIX_FMT_YUV32:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRX1010102:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_BGRA1010102:
		bpp = 32;
		break;
	default:
		return 0;
	}

	/* return bytes required to hold one pixel */
	return (bpp + 7) >> 3;
}

/**
 * vlib_fourcc2bpp - Return media bus string for pixel format
 * @fourcc: Fourcc pixel format code
 *
 * Return: Media bus string for @fourcc or 0.
 */
const char *vlib_fourcc2mbus(uint32_t fourcc)
{
	/* look up mbus format string */
	switch (fourcc) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_UYVY:
		return "UYVY";
	case V4L2_PIX_FMT_BGR24:
	case V4L2_PIX_FMT_RGB24:
	case V4L2_PIX_FMT_BGR32:
	case V4L2_PIX_FMT_ABGR32:
	case V4L2_PIX_FMT_XBGR32:
	case V4L2_PIX_FMT_RGB32:
	case V4L2_PIX_FMT_ARGB32:
	case V4L2_PIX_FMT_XRGB32:
		return "RBG24";
	default:
		return 0;
	}
}

void vlib_drm_drop_master(void)
{
	drmDropMaster(video_setup->drm.fd);
}

int vlib_pipeline_stop_gst(void)
{
	int ret = 0;

	/* Set application state */
	video_setup->app_state = MODE_EXIT;

	if (!(video_setup->flags & VLIB_CFG_FLAG_MULTI_INSTANCE)) {
		// Disable video layer on pipeline stop
		ret |= drm_set_plane_state(&video_setup->drm,
					   video_setup->drm.overlay_plane.drm_plane->plane_id,
					   0);
	}

	return ret;
}

int vlib_init_gst(struct vlib_config_data *cfg)
{
	int ret;
	size_t bpp;

	cfg->buffer_cnt = 0;

	g_mutex_lock (&lock);

	/* Allocate video_setup struct and zero out memory */
	video_setup = calloc (1, sizeof(*video_setup));
	video_setup->app_state = MODE_INIT;
	video_setup->in_fourcc = cfg->fmt_in ? cfg->fmt_in : INPUT_PIX_FMT;
	video_setup->out_fourcc = cfg->fmt_out ? cfg->fmt_out : OUTPUT_PIX_FMT;
	video_setup->flags = cfg->flags;

	bpp = vlib_fourcc2bpp(video_setup->in_fourcc);
	if (!bpp) {
		VLIB_REPORT_ERR("unsupported pixel format '%.4s'",
				(const char *)&video_setup->in_fourcc);
		return VLIB_ERROR_INVALID_PARAM;
	}

	/* Set input resolution */
	video_setup->h = cfg->height_in;
	video_setup->w = cfg->width_in;
	video_setup->stride = video_setup->w * vlib_fourcc2bpp(video_setup->in_fourcc);
	video_setup->fps.numerator = cfg->fps.numerator;
	video_setup->fps.denominator = cfg->fps.denominator;

	/* Skip DRM init if -X flag is set */
	if (cfg->flags & VLIB_CFG_FLAG_MEDIA_EXIT) {
		return VLIB_SUCCESS;
	}

	/* Initialize DRM device */
	ret = vlib_drm_init(cfg);
	if (ret) {
		return ret;
	}

	/* Disable TPG unless input and output resolution match */
	if (video_setup->w != video_setup->w_out ||
	    video_setup->h != video_setup->h_out) {
		vlib_video_src_class_disable(VLIB_VCLASS_TPG);
	}

	/* Set fps to monitor refresh rate if not provided by user */
	if (!video_setup->fps.numerator) {
		size_t vr;
		ret = vlib_drm_try_mode(cfg->display_id, video_setup->w_out, video_setup->h_out, &vr);
		if (ret == VLIB_SUCCESS) {
			video_setup->fps.numerator = vr;
			video_setup->fps.denominator = 1;
		} else {
			return ret;
		}
	}

	return ret;
}

int vlib_uninit_gst(void)
{
	int ret = 0;

	drm_uninit(&video_setup->drm);

	vlib_video_src_uninit();

	free(video_setup);

	return ret;
}

int vlib_change_mode_gst(struct vlib_config *config)
{
	int ret = VLIB_SUCCESS;

	/* Print requested config */
	vlib_dbg("config: src=%zu, type=%d, mode=%zu\n", config->vsrc,
		 config->type, config->mode);

	if (config->vsrc >= vlib_video_src_cnt_get()) {
		VLIB_REPORT_ERR("invalid video source '%zu'",
				config->vsrc);
		return VLIB_ERROR_INVALID_PARAM;
	}

	/* Set application state */
	video_setup->app_state = MODE_CHANGE;

	struct vlib_vdev *vdev = vlib_video_src_get(config->vsrc);
	if (!vdev) {
		return VLIB_ERROR_INVALID_PARAM;
	}

	/*
	 * The user can pass additional information to the video device using
	 * the config.type field which is copied to the vdev.priv field if the
	 * corresponding flag is set by video device.
	 */
	if (vdev->flags & VDEV_CONFIG_TYPE_USER) {
		memcpy(vdev->priv, &config->type, sizeof(config->type));
	}

	/* Set video source */
	video_setup->vid_src = vdev;

	if (vdev->ops && vdev->ops->change_mode) {
		ret = vdev->ops->change_mode(video_setup, config);
		if (ret) {
			return ret;
		}
	}

	/* Configure media pipeline */
	if (vdev->ops && vdev->ops->set_media_ctrl) {
		ret = vdev->ops->set_media_ctrl(video_setup, vdev);
		ASSERT2(!ret, "failed to configure media pipeline\n");
	}

	/* if custom frame rate handler exists call it instead */
	if (video_setup->fps.numerator != 0 &&
	    video_setup->fps.denominator != 0 &&
	    vdev->ops && vdev->ops->set_frame_rate) {
		ret = vdev->ops->set_frame_rate(vdev,
				video_setup->fps.numerator,
				video_setup->fps.denominator);
		if (ret) {
			return ret;
		}
	}

	g_mutex_unlock (&lock);

	return ret;
}

int vlib_get_active_plane_id(void)
{
	if (video_setup->drm.overlay_plane.drm_plane)
		return video_setup->drm.overlay_plane.drm_plane->plane_id;
	else
		return VLIB_ERROR_INVALID_PARAM;
}
