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

#ifndef VDF_LIB_H
#define VDF_LIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include <helper.h>


/* Common interface for video library*/
typedef enum {
	DRM_MODULE_XILINX,
	DRM_MODULE_XYLON,
	DRM_MODULE_NONE = -1,
} drm_module;

/* FIXME: remove global variable */
extern drm_module module_g;

typedef enum {
	VIDEO_CTRL_OFF,
	VIDEO_CTRL_ON,
	VIDEO_CTRL_AUTO
} video_ctrl;

typedef enum {
	CAPTURE_EVENT,
	DISPLAY_EVENT,
	PROCESS_IN_EVENT,
	PROCESS_OUT_EVENT,
	NUM_EVENTS
} pipeline_event;

enum vlib_vsrc_class {
	VLIB_VCLASS_VIVID,
	VLIB_VCLASS_UVC,
	VLIB_VCLASS_TPG,
	VLIB_VCLASS_HDMII,
	VLIB_VCLASS_CSI,
	VLIB_VCLASS_FILE,
	VLIB_VCLASS_GMSL,
};


struct vlib_config {
	size_t vsrc;
	unsigned int type;
	size_t mode;
};

#include <linux/videodev2.h>
#include <common.h>

struct vlib_config_data {
	int width_in;		/* input width */
	int height_in;		/* input height */
	uint32_t fmt_in;	/* input pixel format */
	unsigned int flags;	/* flags */
	unsigned int display_id;	/* display id */
	int width_out;		/* output width */
	int height_out;		/* output height */
	uint32_t fmt_out;	/* output pixel format */
	struct v4l2_fract fps;	/* frames per second */
	const char *vcap_file_fn;	/* filename for file source */
	struct vlib_plane plane;
	size_t vrefresh;	/* vertical refresh rate */
	const char *drm_background;	/* path to background image */
	size_t buffer_cnt;	/* number of frame buffers */
};

#define VLIB_CFG_FLAG_PR_ENABLE		BIT(0) /* enable partial reconfiguration */
#define VLIB_CFG_FLAG_MULTI_INSTANCE	BIT(1) /* enable multi-instance mode */
#define VLIB_CFG_FLAG_MEDIA_EXIT	BIT(2) /* init media pipe and exit */
#define VLIB_CFG_FLAG_FILE_ENABLE	BIT(3) /* enable file source */

/**
 * Error codes. Most vlib functions return 0 on success or one of these
 * codes on failure.
 * User can call vlib_error_name() to retrieve a string representation of an
 * error code or vlib_strerror() to get an end-user suitable description of
 * an error code.
*/

/* Total number of error codes in enum vlib_error */
#define VLIB_ERROR_COUNT 6

typedef enum {
	VLIB_SUCCESS = 0,
	VLIB_ERROR_INTERNAL = -1,
	VLIB_ERROR_CAPTURE = -2,
	VLIB_ERROR_INVALID_PARAM = -3,
	VLIB_ERROR_FILE_IO = -4,
	VLIB_ERROR_NOT_SUPPORTED = -5,
	VLIB_ERROR_NO_MEM = -6,
	VLIB_ERROR_OTHER = -99
} vlib_error;

/* Character-array to store string-representation of the error-codes */
extern char vlib_errstr[];

/**
 *  Log message levels.
 *  - VLIB_LOG_LEVEL_NONE (0)
 *  - VLIB_LOG_LEVEL_ERROR (1)
 *  - VLIB_LOG_LEVEL_WARNING (2)
 *  - VLIB_LOG_LEVEL_INFO (3)
 *  - VLIB_LOG_LEVEL_DEBUG (4)
 *  - VLIB_LOG_LEVEL_EVENT (5)
 *  All the messages are printed on stderr.
 */
typedef enum {
	VLIB_LOG_LEVEL_NONE = 0,
	VLIB_LOG_LEVEL_ERROR,
	VLIB_LOG_LEVEL_WARNING,
	VLIB_LOG_LEVEL_INFO,
	VLIB_LOG_LEVEL_DEBUG,
	VLIB_LOG_LEVEL_EVENT,
} vlib_log_level;

struct video_resolution {
	unsigned int height;
	unsigned int width;
	unsigned int stride;
};

/* The following is used to silence warnings for unused variables */
#define UNUSED(var)		do { (void)(var); } while(0)

/* video source helper functions */
struct vlib_vdev;

const char *vlib_video_src_get_display_text(const struct vlib_vdev *vsrc);
const char *vlib_video_src_get_entity_name(const struct vlib_vdev *vsrc);
enum vlib_vsrc_class vlib_video_src_get_class(const struct vlib_vdev *vsrc);
size_t vlib_video_src_get_index(const struct vlib_vdev *vsrc);
struct vlib_vdev *vlib_video_src_get(size_t id);
const char *video_src_get_vdev_from_id(size_t id);
int vlib_get_active_plane_id(void);
size_t vlib_video_src_cnt_get(void);
int vlib_video_src_init(struct vlib_config_data *cfg);
void vlib_video_src_uninit(void);
int vlib_platform_setup(struct vlib_config_data *cfg);


static inline const char *vlib_video_src_get_display_text_from_id(size_t id)
{
	const struct vlib_vdev *v = vlib_video_src_get(id);
	return vlib_video_src_get_display_text(v);
}

static inline const char *vlib_video_src_get_vdev_from_id(size_t id)
{
	return video_src_get_vdev_from_id(id);
}

static inline const char *vlib_video_src_get_entity_name_from_id(size_t id)
{
	const struct vlib_vdev *v = vlib_video_src_get(id);
	return vlib_video_src_get_entity_name(v);
}

/* drm helper functions */
int vlib_drm_set_layer0_state(int);
int vlib_drm_set_layer0_transparency(int);
int vlib_drm_set_layer0_position(int, int);
int vlib_drm_try_mode(unsigned int display_id, int width, int height,
		      size_t *vrefresh);
void vlib_drm_drop_master(void);

/* video resolution functions */
int vlib_get_active_height(void);
int vlib_get_active_width(void);
unsigned int vlib_get_fps(void);

/* init/uninit functions */
int vlib_init_gst(struct vlib_config_data *cfg);
int vlib_uninit_gst(void);

/* video pipeline control functions */
int vlib_pipeline_stop_gst(void);
int vlib_change_mode_gst(struct vlib_config *config);

void vlib_store_fname_src(const char *file_name);

/* set event-log function */
int vlib_set_event_log(int state);
/* Query pipeline events*/
float vlib_get_event_cnt(pipeline_event event);

/* return the string representation of the error code */
const char *vlib_error_name(vlib_error error_code);
/* return user-readable description of the error-code */
char *vlib_strerror(void);

#ifdef __cplusplus
}
#endif

#endif /* VDF_LIB_H */
