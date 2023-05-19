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

#include <fcntl.h>
#include <glib.h>
#include <glob.h>
#include <mediactl/mediactl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <common.h>
#include <helper.h>
#include <mediactl_helper.h>
#include <vcap_hdmi_int.h>
#include <vcap_file_int.h>
#include <vcap_tpg_int.h>
#include <vcap_csi_int.h>
#include <vcap_gmsl_int.h>
#include <vcap_uvc_int.h>
#include <vcap_vivid_int.h>
#include <video_int.h>

static GPtrArray *video_srcs;

const char *vlib_video_src_get_display_text(const struct vlib_vdev *vsrc)
{
	if (!vsrc) {
		return NULL;
	}

	return vsrc->display_text;
}

const char *vlib_video_src_get_entity_name(const struct vlib_vdev *vsrc)
{
	if (!vsrc) {
		return NULL;
	}

	return vsrc->entity_name;
}

/**
 * vlib_video_src_get_vnode - get video node file descriptor
 * @vsrc:	Pointer to video source struct
 *
 * Obtain the file descriptor for a /dev/videoN node associated with @vsrc.
 *
 * Return: A valid file descriptor on success, -1 otherwise.
 */
int vlib_video_src_get_vnode(const struct vlib_vdev *vsrc)
{
	if (!vsrc) {
		return -1;
	}

	switch (vsrc->vsrc_type) {
	case VSRC_TYPE_MEDIA:
		return vsrc->data.media.vnode;
	case VSRC_TYPE_V4L2:
		return vsrc->data.v4l2.vnode;
	default:
		break;
	}

	return -1;
}

/**
 * vlib_video_src_get_index - get index of a video source
 * @vsrc:	Pointer to video source struct
 *
 * Obtain the unique index associated with @vsrc.
 *
 * Return: The index of @vsrc.
 */
size_t vlib_video_src_get_index(const struct vlib_vdev *vsrc)
{
	ASSERT2(vsrc, "invalid vsrc\n");

	for (size_t i = 0; i < video_srcs->len; ++i) {
		const struct vlib_vdev *tmp = g_ptr_array_index(video_srcs, i);

		if (tmp == vsrc)
			return i;
	}

	ASSERT2(0, "vsrc not found\n");
}

/**
 * vlib_video_src_get_class - get video class
 * @vsrc:	Pointer to video source struct
 *
 * Obtain the video class associated with @vsrc.
 *
 * Return: A valid video_class on success.
 */
enum vlib_vsrc_class vlib_video_src_get_class(const struct vlib_vdev *vsrc)
{
	ASSERT2(vsrc, "invalid vsrc\n");

	return vsrc->vsrc_class;
}

struct vlib_vdev *vlib_video_src_get(size_t id)
{
	if (id >= video_srcs->len) {
		return NULL;
	}

	return g_ptr_array_index(video_srcs, id);
}

struct media_device *vlib_vdev_get_mdev(const struct vlib_vdev *vdev)
{
	if (vdev->vsrc_type != VSRC_TYPE_MEDIA) {
		return NULL;
	}

	return vdev->data.media.mdev;
}

static void vlib_vsrc_vdev_free(struct vlib_vdev *vd)
{
	switch (vd->vsrc_type) {
	case VSRC_TYPE_MEDIA:
		media_device_unref(vd->data.media.mdev);
		close(vd->data.media.vnode);
		break;
	case VSRC_TYPE_V4L2:
		close(vd->data.v4l2.vnode);
		break;
	default:
		break;
	}

	free(vd->priv);
	free(vd);
}

static void vlib_vsrc_table_free_func(void *e)
{
	struct vlib_vdev *vd = e;

	vlib_vsrc_vdev_free(vd);
}

static void vlib_video_src_disable(struct vlib_vdev *vsrc)
{
	g_ptr_array_remove_fast(video_srcs, vsrc);
}

void vlib_video_src_class_disable(enum vlib_vsrc_class class)
{
	for (size_t i = video_srcs->len; i > 0; i--) {
		struct vlib_vdev *vd = g_ptr_array_index(video_srcs, i - 1);

		if (vd->vsrc_class == class) {
			vlib_video_src_disable(vd);
		}
	}
}

static const struct matchtable mt_entities[] = {
	{
		.s = "vcap_tpg output 0", .init = vcap_tpg_init,
	},
	{
		.s = "vcap_hdmi output 0", .init = vcap_hdmi_init,
	},
	{
		.s = "vcap_csi output 0", .init = vcap_csi_init,
	},
	{
		.s = "vcap_gmsl output 0", .init = vcap_gmsl_init,
	}
};

static struct vlib_vdev *init_xvideo(const struct matchtable *mte, void *media)
{
	struct vlib_vdev *vd = NULL;
	size_t nents = media_get_entities_count(media);

	for (size_t i = 0; i < nents; i++) {
		const struct media_entity_desc *info;
		struct media_entity *entity = media_get_entity(media, i);

		if (!entity) {
			vlib_warn("failed to get entity %zu\n", i);
			continue;
		}

		info = media_entity_get_info(entity);
		for (size_t j = 0; j < ARRAY_SIZE(mt_entities); j++) {
			if (!strcmp(mt_entities[j].s, info->name)) {
				vd = mt_entities[j].init(&mt_entities[j], media);
				break;
			}
		}
	}

	return vd;
}

static const struct matchtable mt_drivers_media[] = {
	{
		.s = "xilinx-video", .init = init_xvideo,
	},
#ifdef ENABLE_VCAP_UVC
	{
		.s = "uvcvideo", .init = vcap_uvc_init,
	},
#endif
};

static const struct matchtable mt_drivers_v4l2[] = {
#ifdef ENABLE_VCAP_VIVID
	{
		.s = "vivid", .init = vcap_vivid_init,
	},
#endif
};

int vlib_video_src_init(struct vlib_config_data *cfg)
{
	int ret;
	glob_t pglob;

	video_srcs = g_ptr_array_new_with_free_func(vlib_vsrc_table_free_func);
	if (!video_srcs) {
		return VLIB_ERROR_OTHER;
	}

	ret = glob("/dev/media*", 0, NULL, &pglob);
	if (ret && ret != GLOB_NOMATCH) {
		ret = VLIB_ERROR_OTHER;
		goto error;
	}

	for (size_t i = 0; i < pglob.gl_pathc; i++) {
		struct media_device *media = media_device_new(pglob.gl_pathv[i]);

		if (!media) {
			vlib_warn("failed to create media device from '%s'\n",
				  pglob.gl_pathv[i]);
			continue;
		}

		ret = media_device_enumerate(media);
		if (ret < 0) {
			vlib_warn("failed to enumerate '%s'\n",
				  pglob.gl_pathv[i]);
			media_device_unref(media);
			continue;
		}

		const struct media_device_info *info = media_get_info(media);

		size_t j;
		for (j = 0; j < ARRAY_SIZE(mt_drivers_media); j++) {
			if (strcmp(mt_drivers_media[j].s, info->driver)) {
				continue;
			}

			struct vlib_vdev *vd =
				  mt_drivers_media[j].init(&mt_drivers_media[j],
							   media);
			if (vd) {
				vlib_dbg("found video source '%s (%s)'\n",
					 vd->display_text, pglob.gl_pathv[i]);
				g_ptr_array_add(video_srcs, vd);
				break;
			}
		}

		if (j == ARRAY_SIZE(mt_drivers_media)) {
			media_device_unref(media);
		}
	}

	globfree(&pglob);

	ret = glob("/dev/video*", 0, NULL, &pglob);
	if (ret && ret != GLOB_NOMATCH) {
		ret = VLIB_ERROR_OTHER;
		goto error;
	}

	ret = VLIB_SUCCESS;

	for (size_t i = 0; i < pglob.gl_pathc; i++) {
		int fd = open(pglob.gl_pathv[i], O_RDWR);
		if (fd < 0) {
			ret = VLIB_ERROR_OTHER;
			goto error;
		}

		struct v4l2_capability vcap;
		ret = ioctl(fd, VIDIOC_QUERYCAP, &vcap);
		if (ret) {
			close(fd);
			continue;
		}

		size_t j;
		for (j = 0; j < ARRAY_SIZE(mt_drivers_v4l2); j++) {
			if (strcmp(mt_drivers_v4l2[j].s, (char *)vcap.driver)) {
				continue;
			}

			struct vlib_vdev *vd =
				    mt_drivers_v4l2[j].init(&mt_drivers_v4l2[j],
							    (void *)(uintptr_t)fd);
			if (vd) {
				vlib_dbg("found video source '%s (%s)'\n",
					 vd->display_text, pglob.gl_pathv[i]);
				strcpy(vd->data.v4l2.vdev_name, pglob.gl_pathv[i]);
				g_ptr_array_add(video_srcs, vd);
				break;
			}
		}

		if (j == ARRAY_SIZE(mt_drivers_v4l2)) {
			close(fd);
		}
	}

#ifdef ENABLE_VCAP_FILE
	if (cfg->flags & VLIB_CFG_FLAG_FILE_ENABLE) {
		struct vlib_vdev *vd = vcap_file_init(NULL, (void *)cfg->vcap_file_fn);
		if (vd) {
			vlib_dbg("found video source '%s (%s)'\n",
				 vd->display_text, cfg->vcap_file_fn);
			g_ptr_array_add(video_srcs, vd);
		} else {
			ret = VLIB_ERROR_OTHER;
		}
	}
#endif

error:
	globfree(&pglob);
	return ret;
}

void vlib_video_src_uninit(void)
{
	g_ptr_array_free(video_srcs, TRUE);
}

size_t vlib_video_src_cnt_get(void)
{
	return video_srcs->len;
}

const char *vlib_video_src_mdev2vdev(struct media_device *media)
{
	struct media_entity *ent = media_get_entity(media, 0);
	if (!ent) {
		return NULL;
	}

	return media_entity_get_devname(ent);
}

const char *video_src_get_vdev_from_id(size_t id)
{
	const struct vlib_vdev *v = vlib_video_src_get(id);
	if (v && (v->vsrc_type == VSRC_TYPE_MEDIA))
		return vlib_video_src_mdev2vdev(v->data.media.mdev);
	else {
		size_t i;
		for (i = 0; i < ARRAY_SIZE(mt_drivers_v4l2); i++) {
			if (strcmp(mt_drivers_v4l2[i].s, vlib_video_src_get_entity_name(v)))
				continue;
			return v->data.v4l2.vdev_name;
		}
	}

	return '\0';
}
