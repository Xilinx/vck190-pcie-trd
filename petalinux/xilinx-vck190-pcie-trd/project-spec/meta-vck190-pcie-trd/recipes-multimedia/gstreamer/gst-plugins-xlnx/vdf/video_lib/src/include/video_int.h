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

#ifndef VIDEO_INT_H
#define VIDEO_INT_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Misc configuration */
#define OUTPUT_PIX_FMT v4l2_fourcc('Y','U','Y','V')
#define INPUT_PIX_FMT  v4l2_fourcc('Y','U','Y','V')

#define POLL_TIMEOUT_MSEC 5000

#define DRI_CARD_DP	0
#define DRI_CARD_HDMI	1

struct media_device;
struct video_pipeline;
struct vlib_vdev;

#include <video.h>

/**
 * struct matchtable:
 * @s: String to match compatible items against
 * @init: Init function
 *	  @mte: Match table entry that matched @s.
 *	  @data: Custom data pointer.
 *	  Return: struct vlib_vdev on success,
 *		  NULL for unsupported/invalid input
 */
struct matchtable {
	char *s;
	struct vlib_vdev *(*init)(const struct matchtable *mte, void *data);
};

struct vsrc_ops {
	int (*change_mode)(struct video_pipeline *video_setup,
			   struct vlib_config *config);
	int (*set_media_ctrl)(struct video_pipeline *video_setup,
			      const struct vlib_vdev *vdev);
	int (*set_frame_rate)(const struct vlib_vdev *vdev, size_t numerator, size_t denominator);
};

struct vlib_vdev {
	enum vlib_vsrc_class vsrc_class;
	const char *display_text;
	const char *entity_name;
	union {
		struct {
			struct media_device *mdev;
			int vnode;		/* video node file descriptor */
		} media;
		struct {
			char vdev_name[DEV_NAME_LEN];	/* video node name */
			int vnode;		/* video node file descriptor */
		} v4l2;
		struct {
			FILE *fd;
			const char *filename;
		} file;
	} data;
	enum {
		VSRC_TYPE_INVALID,
		VSRC_TYPE_MEDIA,
		VSRC_TYPE_V4L2,
		VSRC_TYPE_FILE,
	} vsrc_type;
	const struct vsrc_ops *ops;
	unsigned int flags;
	void *priv;
};

/* This flag allows the user to pass additional data to the video device through
 * the config.type field. This data gets stored in the vdev.priv field. If the
 * vdev.priv field is used by the device itself, no user data can be passed.
 */
#define VDEV_CONFIG_TYPE_USER	BIT(0)

typedef enum {
	MODE_INIT,
	MODE_CHANGE,
	MODE_EXIT
} app_state;


#include <drm_helper.h>

/* global setup for all modes */
struct video_pipeline {
	/* input */
	unsigned int w, h; /* input width, height */
	unsigned int stride; /* input stride */
	unsigned int in_fourcc; /* input pixel format */
	struct v4l2_fract fps; /* frame rate */
	/* output */
	unsigned int w_out, h_out; /* output width, height */
	unsigned int vtotal, htotal;
	unsigned int stride_out; /* output stride */
	unsigned int out_fourcc; /* output pixel format */
	struct drm_device drm;
	/* current state */
	int app_state;
	const struct vlib_vdev *vid_src;
	unsigned int flags;
};

int vlib_video_src_init(struct vlib_config_data *cfg);
void vlib_video_src_uninit(void);
struct media_device *vlib_vdev_get_mdev(const struct vlib_vdev *vdev);
void vlib_video_src_class_disable(enum vlib_vsrc_class class);
const char *vlib_video_src_mdev2vdev(struct media_device *media);
int vlib_video_src_get_vnode(const struct vlib_vdev *vsrc);
size_t vlib_fourcc2bpp(uint32_t fourcc);
const char *vlib_fourcc2mbus(uint32_t fourcc);
int vlib_platform_set_qos(size_t qos_setting);

void vlib_log(vlib_log_level level, const char *format, ...)
		__attribute__((__format__(__printf__, 2, 3)));
void vlib_log_v(vlib_log_level level, const char *format, va_list args);

#define VLIB_ERRSTR_SIZE 256
#define _vlib_log(level, ...) vlib_log(level, __VA_ARGS__)

#define VLIB_REPORT_ERR(fmt, ...) \
		snprintf(vlib_errstr, VLIB_ERRSTR_SIZE, fmt, ## __VA_ARGS__)

#ifdef DEBUG_MODE
#define INFO_MODE
#define WARN_MODE
#define ERROR_MODE
#define vlib_dbg(...) _vlib_log(VLIB_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define vlib_dbg(...) do {} while(0)
#endif

#ifdef INFO_MODE
#define WARN_MODE
#define ERROR_MODE
#define vlib_info(...) _vlib_log(VLIB_LOG_LEVEL_INFO, __VA_ARGS__)
#else
#define vlib_info(...) do {} while(0)
#endif

#ifdef WARN_MODE
#define ERROR_MODE
#define vlib_warn(...) _vlib_log(VLIB_LOG_LEVEL_WARNING, __VA_ARGS__)
#else
#define vlib_warn(...) do {} while(0)
#endif

#ifdef ERROR_MODE
#define vlib_err(...) _vlib_log(VLIB_LOG_LEVEL_ERROR, __VA_ARGS__)
#else
#define vlib_err(...) do {} while(0)
#endif

#define vlib_event(...) _vlib_log(VLIB_LOG_LEVEL_EVENT, __VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* VIDEO_INT_H */
