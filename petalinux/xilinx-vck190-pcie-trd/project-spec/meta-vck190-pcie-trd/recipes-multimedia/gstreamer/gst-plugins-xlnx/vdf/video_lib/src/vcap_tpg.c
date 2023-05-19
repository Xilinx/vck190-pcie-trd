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
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <mediactl/mediactl.h>
#include <mediactl/v4l2subdev.h>
#include <unistd.h>

#include <helper.h>
#include <mediactl_helper.h>
#include <v4l2_helper.h>
#include <vcap_tpg_int.h>

#if defined(PLATFORM_ZCU102) || defined (PLATFORM_ZC1751_DC1)
#define MEDIA_TPG_ENTITY "b0030000.tpg"
#elif defined(PLATFORM_ZC70X)
#define MEDIA_TPG_ENTITY "40080000.tpg"
#endif
#define MEDIA_TPG_FMT_IN "UYVY"

#define TPG_BG_PATTERN_DEFAULT 14
#define TPG_BG_ZPLATE_HORIZONTAL_START_DEFAULT 30
#define TPG_BG_ZPLATE_HORIZONTAL_SPEED_DEFAULT 0
#define TPG_BG_ZPLATE_VERTICAL_START_DEFAULT 1
#define TPG_BG_ZPLATE_VERTICAL_SPEED_DEFAULT 0

#define TPG_FG_DEFAULT 1
#define TPG_FG_BOX_SIZE_DEFAULT 150
#define TPG_FG_BOX_COLOR_DEFAULT 0
#define TPG_FG_BOX_SPEED_DEFAULT 4
#define TPG_FG_CROSS_HAIR_ROWS_DEFAULT 100
#define TPG_FG_CROSS_HAIR_COLUMNS_DEFAULT 100

static char *tpg_pattern_menu_names[TPG_BG_PATTERN_CNT];
static unsigned int bg_pattern = TPG_BG_PATTERN_DEFAULT;
static unsigned int fg_pattern = TPG_FG_DEFAULT;
static unsigned int box_size = TPG_FG_BOX_SIZE_DEFAULT;
static unsigned int box_color = TPG_FG_BOX_COLOR_DEFAULT;
static unsigned int box_speed = TPG_FG_BOX_SPEED_DEFAULT;
static unsigned int cross_hair_row = TPG_FG_CROSS_HAIR_ROWS_DEFAULT;
static unsigned int cross_hair_column = TPG_FG_CROSS_HAIR_COLUMNS_DEFAULT;
static unsigned int zplate_hor_start = TPG_BG_ZPLATE_HORIZONTAL_START_DEFAULT;
static unsigned int zplate_hor_speed = TPG_BG_ZPLATE_HORIZONTAL_SPEED_DEFAULT;
static unsigned int zplate_ver_start = TPG_BG_ZPLATE_VERTICAL_START_DEFAULT;
static unsigned int zplate_ver_speed = TPG_BG_ZPLATE_VERTICAL_SPEED_DEFAULT;

static int v4l2_tpg_set_ctrl(const struct vlib_vdev *vd, int id, int value)
{
	return v4l2_set_ctrl(vd, MEDIA_TPG_ENTITY, id, value);
}

const char *tpg_get_pattern_menu_name(unsigned int idx)
{
	ASSERT2(idx < TPG_BG_PATTERN_CNT, "Invalid test pattern index\r");
	return tpg_pattern_menu_names[idx];
}

static void tpg_init_pattern_menu_names(const struct vlib_vdev *vdev)
{
	struct v4l2_queryctrl query;
	struct v4l2_querymenu menu;
	char subdev_name[DEV_NAME_LEN];
	int ret, fd;

	get_entity_devname(vlib_vdev_get_mdev(vdev), MEDIA_TPG_ENTITY,
			   subdev_name);

	fd = open(subdev_name, O_RDWR);
	ASSERT2(fd >= 0, "failed to open %s: %s\n", subdev_name, ERRSTR);

	/* query control */
	memset(&query, 0, sizeof(query));
	query.id = V4L2_CID_TEST_PATTERN;
	ret = ioctl(fd, VIDIOC_QUERYCTRL, &query);
	ASSERT2(ret >= 0, "VIDIOC_QUERYCTRL failed: %s\n", ERRSTR);

	for (size_t i = 0; i < TPG_BG_PATTERN_CNT; i++)
		tpg_pattern_menu_names[i] = malloc(32 * sizeof(**tpg_pattern_menu_names));

	/* query menu */
	memset(&menu, 0, sizeof(menu));
	menu.id = query.id;
	for (menu.index = query.minimum; menu.index <= (unsigned)query.maximum; menu.index++) {
		ret = ioctl(fd, VIDIOC_QUERYMENU, &menu);
		if (ret < 0)
			continue;

		strncpy(tpg_pattern_menu_names[menu.index], (char *)menu.name, 32);
	}

	close(fd);
}

void tpg_set_bg_pattern(const struct vlib_vdev *vd, unsigned int bg)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_TEST_PATTERN, bg);
	bg_pattern = bg;
}

void tpg_set_fg_pattern(const struct vlib_vdev *vd, unsigned int fg)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_HLS_FG_PATTERN, fg);
	fg_pattern = fg;
}

void tpg_set_box_size(const struct vlib_vdev *vd, unsigned int size)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_BOX_SIZE, size);
	box_size = size;
}

void tpg_set_box_color(const struct vlib_vdev *vd, unsigned int color)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_BOX_COLOR, color);
	box_color = color;
}

void tpg_set_box_speed(const struct vlib_vdev *vd, unsigned int speed)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_MOTION_SPEED, speed);
	box_speed = speed;
}

void tpg_set_cross_hair_num_rows(const struct vlib_vdev *vd, unsigned int row)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_CROSS_HAIR_COLUMN, row);
	cross_hair_row = row;
}

void tpg_set_cross_hair_num_columns(const struct vlib_vdev *vd, unsigned int column)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_CROSS_HAIR_ROW, column);
	cross_hair_column = column;
}

void tpg_set_zplate_hor_cntl_start(const struct vlib_vdev *vd, unsigned int hstart)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_ZPLATE_HOR_START, hstart);
	zplate_hor_start = hstart;
}

void tpg_set_zplate_hor_cntl_delta(const struct vlib_vdev *vd, unsigned int hspeed)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_ZPLATE_HOR_SPEED, hspeed);
	zplate_hor_speed = hspeed;
}

void tpg_set_zplate_ver_cntl_start(const struct vlib_vdev *vd, unsigned int vstart)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_ZPLATE_VER_START, vstart);
	zplate_ver_start = vstart;
}

void tpg_set_zplate_ver_cntl_delta(const struct vlib_vdev *vd, unsigned int vspeed)
{
	v4l2_tpg_set_ctrl(vd, V4L2_CID_XILINX_TPG_ZPLATE_VER_SPEED, vspeed);
	zplate_ver_speed = vspeed;
}

static void tpg_set_cur_config(const struct vlib_vdev *vd)
{
	/* Set current TPG config */
	tpg_set_bg_pattern(vd, bg_pattern);
	tpg_set_fg_pattern(vd, fg_pattern);
	tpg_set_box_size(vd, box_size);
	tpg_set_box_color(vd, box_color);
	tpg_set_box_speed(vd, box_speed);
	tpg_set_cross_hair_num_rows(vd, cross_hair_row);
	tpg_set_cross_hair_num_columns(vd, cross_hair_column);
	tpg_set_zplate_hor_cntl_start(vd, zplate_hor_start);
	tpg_set_zplate_hor_cntl_delta(vd, zplate_hor_speed);
	tpg_set_zplate_ver_cntl_start(vd, zplate_ver_start);
	tpg_set_zplate_ver_cntl_delta(vd, zplate_ver_speed);
}

static int vcap_tpg_ops_set_media_ctrl(struct video_pipeline *video_setup,
				       const struct vlib_vdev *vdev)
{
	int ret;
	char fmt_str[100];
	struct media_device *media = vlib_vdev_get_mdev(vdev);

	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	ASSERT2(ret >= 0, "failed to enumerate %s\n", vdev->display_text);

#ifdef VLIB_LOG_LEVEL_DEBUG
	const struct media_device_info *info = media_get_info(media);
	print_media_info(info);
#endif

	/* Set TPG input resolution */
	memset(fmt_str, 0, sizeof (fmt_str));
	media_set_fmt_str(fmt_str, MEDIA_TPG_ENTITY, 0, MEDIA_TPG_FMT_IN,
			  video_setup->w, video_setup->h);
	ret = v4l2_subdev_parse_setup_formats(media, fmt_str);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);

	return ret;
}

static int vcap_tpg_ops_change_mode(struct video_pipeline *video_setup,
				    struct vlib_config *config)
{
	const struct vlib_vdev *vd = video_setup->vid_src;
	tpg_set_cur_config(vd);

	return 0;
}

static int vcap_tpg_ops_set_frame_rate(const struct vlib_vdev *vdev,
				       size_t numerator, size_t denominator)
{
	int fd, ret;
	char subdev_name[DEV_NAME_LEN];
	struct v4l2_subdev_frame_interval ival;

	get_entity_devname(vlib_vdev_get_mdev(vdev), MEDIA_TPG_ENTITY,
			   subdev_name);

	fd = open(subdev_name, O_RDWR);
	if (fd <= 0) {
		VLIB_REPORT_ERR("error opening device '%s'", subdev_name);
		return VLIB_ERROR_FILE_IO;
	}

	memset(&ival, 0, sizeof(ival));
	ival.interval.numerator = denominator;
	ival.interval.denominator = numerator;
	ret = ioctl(fd, VIDIOC_SUBDEV_S_FRAME_INTERVAL, &ival);
	if (ret < 0) {
		VLIB_REPORT_ERR("VIDIOC_SUBDEV_S_FRAME_INTERVAL failed");
		goto err;
	}

	vlib_info("frame rate set to: %u/%u fps\n", ival.interval.denominator,
		  ival.interval.numerator);

err:
	close(fd);

	return ret;
}

static const struct vsrc_ops vcap_tpg_ops = {
	.change_mode = vcap_tpg_ops_change_mode,
	.set_media_ctrl = vcap_tpg_ops_set_media_ctrl,
	.set_frame_rate = vcap_tpg_ops_set_frame_rate,
};

struct vlib_vdev *vcap_tpg_init(const struct matchtable *mte, void *media)
{
	struct vlib_vdev *vd = calloc(1, sizeof(*vd));
	if (!vd) {
		return NULL;
	}

	vd->vsrc_type = VSRC_TYPE_MEDIA;
	vd->data.media.mdev = media;
	vd->vsrc_class = VLIB_VCLASS_TPG;
	vd->display_text = "Test Pattern Generator";
	vd->entity_name = mte->s;
	vd->ops = &vcap_tpg_ops;

	vd->data.media.vnode = open(vlib_video_src_mdev2vdev(vd->data.media.mdev), O_RDWR);
	if (vd->data.media.vnode < 0) {
		free(vd);
		return NULL;
	}

	tpg_init_pattern_menu_names(vd);

	return vd;
}
