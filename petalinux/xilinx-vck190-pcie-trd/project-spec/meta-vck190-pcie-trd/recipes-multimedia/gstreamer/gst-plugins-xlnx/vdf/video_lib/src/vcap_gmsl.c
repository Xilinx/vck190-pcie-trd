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
#include <glob.h>
#include <mediactl/mediactl.h>
#include <mediactl/v4l2subdev.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <helper.h>
#include <mediactl_helper.h>
#include <v4l2_helper.h>
#include <vcap_gmsl_int.h>

#define MEDIA_SENSOR1_ENTITY	"AR0231.%u-0011"
#define MEDIA_SENSOR2_ENTITY	"AR0231.%u-0012"
#define MEDIA_SENSOR3_ENTITY	"AR0231.%u-0013"
#define MEDIA_SENSOR4_ENTITY	"AR0231.%u-0014"
#define MEDIA_SERDES_ENTITY	"MAX9286-SERDES.%u-0048"

#define MEDIA_SENSOR_FMT_OUT	"SGRBG8"

#define MEDIA_SERDES_FMT_IN	MEDIA_SENSOR_FMT_OUT
#define MEDIA_SERDES_FMT_OUT	MEDIA_SERDES_FMT_IN

#define MEDIA_GMSL_ENTITY	"a0060000.csiss"
#define MEDIA_GMSL_FMT_IN	MEDIA_SERDES_FMT_OUT
#define MEDIA_GMSL_FMT_OUT	MEDIA_GMSL_FMT_IN

#define MEDIA_AXI4SS_ENTITY	"amba:axis_switch@0"
#define MEDIA_AXI4SS_FMT_IN	MEDIA_GMSL_FMT_IN
#define MEDIA_AXI4SS_FMT_OUT	MEDIA_GMSL_FMT_OUT

#define MEDIA_DMSC0_ENTITY	"b0040000.v_demosaic"
#define MEDIA_DMSC0_FMT_IN	MEDIA_GMSL_FMT_OUT
#define MEDIA_DMSC0_FMT_OUT	"RBG24"

#define MEDIA_SCALER0_ENTITY	"b0080000.scaler"
#define MEDIA_SCALER0_FMT_IN	MEDIA_DMSC0_FMT_OUT

#define MEDIA_DMSC1_ENTITY	"b1040000.v_demosaic"
#define MEDIA_DMSC1_FMT_IN	MEDIA_GMSL_FMT_OUT
#define MEDIA_DMSC1_FMT_OUT	"RBG24"

#define MEDIA_SCALER1_ENTITY	"b1080000.scaler"
#define MEDIA_SCALER1_FMT_IN	MEDIA_DMSC1_FMT_OUT

#define MEDIA_DMSC2_ENTITY	"b2040000.v_demosaic"
#define MEDIA_DMSC2_FMT_IN	MEDIA_GMSL_FMT_OUT
#define MEDIA_DMSC2_FMT_OUT	"RBG24"

#define MEDIA_SCALER2_ENTITY	"b2080000.scaler"
#define MEDIA_SCALER2_FMT_IN	MEDIA_DMSC2_FMT_OUT

#define MEDIA_DMSC3_ENTITY	"b3040000.v_demosaic"
#define MEDIA_DMSC3_FMT_IN	MEDIA_GMSL_FMT_OUT
#define MEDIA_DMSC3_FMT_OUT	"RBG24"

#define MEDIA_SCALER3_ENTITY	"b3080000.scaler"
#define MEDIA_SCALER3_FMT_IN	MEDIA_DMSC3_FMT_OUT

#define GMSL_ACT_LANES		4

static unsigned int act_lanes = GMSL_ACT_LANES;

static int v4l2_gmsl_set_ctrl(const struct vlib_vdev *vd, int id, int value)
{
	return v4l2_set_ctrl(vd, MEDIA_GMSL_ENTITY, id, value);
}

static void gmsl_set_act_lanes(const struct vlib_vdev *vd, unsigned int lanes)
{
	v4l2_gmsl_set_ctrl(vd, V4L2_CID_XILINX_MIPICSISS_ACT_LANES, lanes);
	act_lanes = lanes;
}

#define AR0231AT_VERTICAL_FLIP		0
#define AR0231AT_HORIZONTAL_FLIP	0
#define AR0231AT_TEST_PATTERN		0
#define AR0231AT_EXPOSURE		878
#define AR0231AT_ANALOG_GAIN		5
#define AR0231AT_DIGITAL_GAIN		606
#define AR0231AT_COLOR_GAIN_RED		856
#define AR0231AT_COLOR_GAIN_GREEN	1401
#define AR0231AT_COLOR_GAIN_BLUE	606

static unsigned int exposure[GMSL_NUM_SENSORS] = { AR0231AT_EXPOSURE };
static unsigned int analog_gain[GMSL_NUM_SENSORS] = { AR0231AT_ANALOG_GAIN };
static unsigned int digital_gain[GMSL_NUM_SENSORS] = { AR0231AT_DIGITAL_GAIN };
static unsigned int color_gain_red[GMSL_NUM_SENSORS] = { AR0231AT_COLOR_GAIN_RED };
static unsigned int color_gain_green[GMSL_NUM_SENSORS] = { AR0231AT_COLOR_GAIN_GREEN };
static unsigned int color_gain_blue[GMSL_NUM_SENSORS] = { AR0231AT_COLOR_GAIN_BLUE };
static unsigned int vertical_flip[GMSL_NUM_SENSORS] = { AR0231AT_VERTICAL_FLIP };
static unsigned int horizontal_flip[GMSL_NUM_SENSORS] = { AR0231AT_HORIZONTAL_FLIP };
static unsigned int test_pattern[GMSL_NUM_SENSORS] = { AR0231AT_TEST_PATTERN };
static char *ar0231at_test_pattern_names[AR0231AT_TEST_PATTERN_CNT];

static char sensor_entity[GMSL_NUM_SENSORS][32];
static char serdes_entity[32];

static int v4l2_sensor_set_ctrl(const struct vlib_vdev *vd, unsigned int n,
				int id, int value)
{
	return v4l2_set_ctrl(vd, sensor_entity[n], id, value);
}

void ar0231at_set_exposure(const struct vlib_vdev *vd, unsigned int n,
			   unsigned int exp)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_EXPOSURE, exp);
	exposure[n] = exp;
}

void ar0231at_set_analog_gain(const struct vlib_vdev *vd, unsigned int n,
			      unsigned int gn)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_ANALOGUE_GAIN, gn);
	analog_gain[n] = gn;
}

void ar0231at_set_digital_gain(const struct vlib_vdev *vd, unsigned int n,
			       unsigned int gn)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_GAIN, gn);
	digital_gain[n] = gn;
}

void ar0231at_set_color_gain_red(const struct vlib_vdev *vd, unsigned int n,
				 unsigned int gn)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_RED_BALANCE, gn);
	color_gain_red[n] = gn;
}

void ar0231at_set_color_gain_green(const struct vlib_vdev *vd, unsigned int n,
				   unsigned int gn)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_CHROMA_GAIN, gn);
	color_gain_green[n] = gn;
}

void ar0231at_set_color_gain_blue(const struct vlib_vdev *vd, unsigned int n,
				  unsigned int gn)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_BLUE_BALANCE, gn);
	color_gain_blue[n] = gn;
}

void ar0231at_set_vertical_flip(const struct vlib_vdev *vd, unsigned int n,
				unsigned int vflip)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_VFLIP, vflip);
	vertical_flip[n] = vflip;
}

void ar0231at_set_horizontal_flip(const struct vlib_vdev *vd, unsigned int n,
				  unsigned int hflip)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_HFLIP, hflip);
	horizontal_flip[n] = hflip;
}

void ar0231at_set_test_pattern(const struct vlib_vdev *vd, unsigned int n,
			       unsigned int tp)
{
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");
	v4l2_sensor_set_ctrl(vd, n, V4L2_CID_TEST_PATTERN, tp);
	test_pattern[n] = tp;
}

const char *ar0231at_get_test_pattern_name(unsigned int idx)
{
	ASSERT2(idx < AR0231AT_TEST_PATTERN_CNT, "Invalid test pattern index\r");
	return ar0231at_test_pattern_names[idx];
}

static void ar0231at_init_test_pattern_names(const struct vlib_vdev *vdev)
{
	struct v4l2_queryctrl query;
	struct v4l2_querymenu menu;
	char subdev_name[DEV_NAME_LEN];
	int ret, fd;

	/* hard-code sensor 0 here as all sensors are identical */
	get_entity_devname(vlib_vdev_get_mdev(vdev), sensor_entity[0],
			   subdev_name);

	fd = open(subdev_name, O_RDWR);
	ASSERT2(fd >= 0, "failed to open %s: %s\n", subdev_name, ERRSTR);

	/* query control */
	memset(&query, 0, sizeof(query));
	query.id = V4L2_CID_TEST_PATTERN;
	ret = ioctl(fd, VIDIOC_QUERYCTRL, &query);
	ASSERT2(ret >= 0, "VIDIOC_QUERYCTRL failed: %s\n", ERRSTR);

	for (size_t i = 0; i < AR0231AT_TEST_PATTERN_CNT; i++)
		ar0231at_test_pattern_names[i] = malloc(32 * sizeof(**ar0231at_test_pattern_names));

	/* query menu */
	memset(&menu, 0, sizeof(menu));
	menu.id = query.id;
	for (menu.index = query.minimum; menu.index <= (unsigned)query.maximum; menu.index++) {
		ret = ioctl(fd, VIDIOC_QUERYMENU, &menu);
		if (ret < 0)
			continue;

		strncpy(ar0231at_test_pattern_names[menu.index], (char *)menu.name, 32);
	}

	close(fd);
}

static void __attribute__((__unused__)) gmsl_log_status(const struct vlib_vdev *vdev)
{
        int fd, ret;
        char subdev_name[DEV_NAME_LEN];

        get_entity_devname(vlib_vdev_get_mdev(vdev), MEDIA_GMSL_ENTITY,
			   subdev_name);

        fd = open(subdev_name, O_RDWR);
        ASSERT2(fd >= 0, "failed to open %s: %s\n", subdev_name, ERRSTR);

        ret = ioctl(fd, VIDIOC_LOG_STATUS);
        ASSERT2(ret >= 0, "VIDIOC_LOG_STATUS failed: %s\n", ERRSTR);

        close(fd);
}

static size_t sq_err(size_t w0, size_t h0, size_t w1, size_t h1)
{
	if (w0 < w1) {
		size_t tmp = w0;
		w0 = w1;
		w1 = tmp;
	}
	if (h0 < h1) {
		size_t tmp = h0;
		h0 = h1;
		h1 = tmp;
	}

	size_t err_w = w0 - w1;
	err_w *= err_w;

	size_t err_h = h0 - h1;
	err_h *= err_h;

	return err_w + err_h;
}

/**
 * vcap_gmsl_find_sensor_res - Find best sensor resolution
 * @width: Desired width, updated to best width
 * @height: Desired height, updated to best height
 *
 * Find the best possible sensor resolution for the resolution passed in
 * @widhtx@height. @width and @height are updated to the best supported
 * resolution.
 */
static void vcap_gmsl_find_sensor_res(size_t *width, size_t *height)
{
	size_t err_720 = sq_err(1280, 720, *width, *height);
	size_t err_1080 = sq_err(1920, 1080, *width, *height);

	size_t err = err_720;
	*width = 1280;
	*height = 720;
	if (err_1080 < err) {
		err = err_1080;
		*width = 1920;
		*height = 1080;
	}
}

static int vcap_gmsl_ops_set_media_ctrl(struct video_pipeline *video_setup,
				       const struct vlib_vdev *vdev)
{
	int ret;
	char media_formats[100];
	struct media_device *media = vlib_vdev_get_mdev(vdev);

	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	ASSERT2(ret >= 0, "failed to enumerate %s\n", vdev->display_text);

#ifdef VLIB_LOG_LEVEL_DEBUG
	const struct media_device_info *info = media_get_info(media);
	print_media_info(info);
#endif

	size_t sensor_width = video_setup->w;
	size_t sensor_height = video_setup->h;

	vcap_gmsl_find_sensor_res(&sensor_width, &sensor_height);

	/* Set image sensor format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, sensor_entity[0], 0,
			  MEDIA_SENSOR_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		sensor_entity[0], 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, sensor_entity[1], 0,
			  MEDIA_SENSOR_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		sensor_entity[1], 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, sensor_entity[2], 0,
			  MEDIA_SENSOR_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		sensor_entity[2], 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, sensor_entity[3], 0,
			  MEDIA_SENSOR_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		sensor_entity[3], 0, strerror(-ret), -ret);

	/* Set MAX9286-SERDES format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, serdes_entity, 0,
			  MEDIA_SERDES_FMT_IN, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		serdes_entity, 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, serdes_entity, 1,
			  MEDIA_SERDES_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		serdes_entity, 1, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, serdes_entity, 2,
			  MEDIA_SERDES_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		serdes_entity, 2, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, serdes_entity, 3,
			  MEDIA_SERDES_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		serdes_entity, 3, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, serdes_entity, 4,
			  MEDIA_SERDES_FMT_OUT, sensor_width,
			  (GMSL_NUM_SENSORS*sensor_height));
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		serdes_entity, 4, strerror(-ret), -ret);

	/* Set MIPI CSI2 Rx format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_GMSL_ENTITY, 0,
			  MEDIA_GMSL_FMT_IN, sensor_width,
			  (GMSL_NUM_SENSORS*sensor_height));
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_GMSL_ENTITY, 0, strerror(-ret), -ret);

	/* Set AXI4-Stream Switch format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_AXI4SS_ENTITY, 0,
			  MEDIA_AXI4SS_FMT_IN, sensor_width,
			  (GMSL_NUM_SENSORS*sensor_height));
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_AXI4SS_ENTITY, 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_AXI4SS_ENTITY, 1,
			  MEDIA_AXI4SS_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_AXI4SS_ENTITY, 1, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_AXI4SS_ENTITY, 2,
			  MEDIA_AXI4SS_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_AXI4SS_ENTITY, 2, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_AXI4SS_ENTITY, 3,
			  MEDIA_AXI4SS_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_AXI4SS_ENTITY, 3, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_AXI4SS_ENTITY, 4,
			  MEDIA_AXI4SS_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_AXI4SS_ENTITY, 4, strerror(-ret), -ret);

	/* Set Demosaic format */
	/* demosaic_0 */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC0_ENTITY, 0,
			  MEDIA_DMSC0_FMT_IN, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_DMSC0_ENTITY, 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC0_ENTITY, 1,
			  MEDIA_DMSC0_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_DMSC0_ENTITY, 1, strerror(-ret), -ret);
	/* demosaic_1 */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC1_ENTITY, 0,
			  MEDIA_DMSC1_FMT_IN, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_DMSC1_ENTITY, 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC1_ENTITY, 1,
			  MEDIA_DMSC1_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_DMSC1_ENTITY, 1, strerror(-ret), -ret);
	/* demosaic_2 */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC2_ENTITY, 0,
			  MEDIA_DMSC2_FMT_IN, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_DMSC2_ENTITY, 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC2_ENTITY, 1,
			  MEDIA_DMSC2_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_DMSC2_ENTITY, 1, strerror(-ret), -ret);
	/* demosaic_3 */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC3_ENTITY, 0,
			  MEDIA_DMSC3_FMT_IN, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_DMSC3_ENTITY, 0, strerror(-ret), -ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC3_ENTITY, 1,
			  MEDIA_DMSC3_FMT_OUT, sensor_width, sensor_height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
		MEDIA_DMSC3_ENTITY, 1, strerror(-ret), -ret);

	/* Set Scaler format based on selected video node */
	unsigned int n = *(unsigned int *)vdev->priv;
	ASSERT2(n < GMSL_NUM_SENSORS, "Sensor index out of bounds\r");

	switch (n) {
	case 0:
		/* scaler_0 */
		memset(media_formats, 0, sizeof(media_formats));
		media_set_fmt_str(media_formats, MEDIA_SCALER0_ENTITY, 0,
				  MEDIA_SCALER0_FMT_IN, sensor_width, sensor_height);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
			MEDIA_SCALER0_ENTITY, 0, strerror(-ret), -ret);
		memset(media_formats, 0, sizeof(media_formats));
		media_set_fmt_str(media_formats, MEDIA_SCALER0_ENTITY, 1,
				  vlib_fourcc2mbus(video_setup->in_fourcc),
				  video_setup->w, video_setup->h);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
			MEDIA_SCALER0_ENTITY, 1, strerror(-ret), -ret);
		break;
	case 1:
		/* scaler_1 */
		memset(media_formats, 0, sizeof(media_formats));
		media_set_fmt_str(media_formats, MEDIA_SCALER1_ENTITY, 0,
				  MEDIA_SCALER1_FMT_IN, sensor_width, sensor_height);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
			MEDIA_SCALER1_ENTITY, 0, strerror(-ret), -ret);
		memset(media_formats, 0, sizeof(media_formats));
		media_set_fmt_str(media_formats, MEDIA_SCALER1_ENTITY, 1,
				  vlib_fourcc2mbus(video_setup->in_fourcc),
				  video_setup->w, video_setup->h);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
			MEDIA_SCALER1_ENTITY, 1, strerror(-ret), -ret);
		break;
	case 2:
		/* scaler_2 */
		memset(media_formats, 0, sizeof(media_formats));
		media_set_fmt_str(media_formats, MEDIA_SCALER2_ENTITY, 0,
				  MEDIA_SCALER2_FMT_IN, sensor_width, sensor_height);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
			MEDIA_SCALER2_ENTITY, 0, strerror(-ret), -ret);
		memset(media_formats, 0, sizeof(media_formats));
		media_set_fmt_str(media_formats, MEDIA_SCALER2_ENTITY, 1,
				  vlib_fourcc2mbus(video_setup->in_fourcc),
				  video_setup->w, video_setup->h);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
			MEDIA_SCALER2_ENTITY, 1, strerror(-ret), -ret);
		break;
	case 3:
		/* scaler_3 */
		memset(media_formats, 0, sizeof(media_formats));
		media_set_fmt_str(media_formats, MEDIA_SCALER3_ENTITY, 0,
				  MEDIA_SCALER3_FMT_IN, sensor_width, sensor_height);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
			MEDIA_SCALER3_ENTITY, 0, strerror(-ret), -ret);
		memset(media_formats, 0, sizeof(media_formats));
		media_set_fmt_str(media_formats, MEDIA_SCALER3_ENTITY, 1,
				  vlib_fourcc2mbus(video_setup->in_fourcc),
				  video_setup->w, video_setup->h);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		ASSERT2(!ret, "Unable to setup formats for %s pad%d: %s (%d)\n",
			MEDIA_SCALER3_ENTITY, 1, strerror(-ret), -ret);
		break;
	}

	return ret;
}

static int ar0231at_get_i2cbus(unsigned int *i2cbus)
{
	int ret;
	glob_t pglob;
	char *split;

	/* Check i2c bus number of sensor 0 */
	ret = glob("/sys/devices/platform/amba/*.i2c/i2c-*/i2c-*/*-0011",
		   0, NULL, &pglob);
	if (ret || pglob.gl_pathc != 1) {
		VLIB_REPORT_ERR("No AR0231 sensor device found!");
		ret = VLIB_ERROR_OTHER;
		goto error;
	}

	do {
		split = strsep(&pglob.gl_pathv[0], "/");
	} while(pglob.gl_pathv[0] != NULL);

	sscanf(split, "%u-0011", i2cbus);
	vlib_info("AR0231 sensor detected on i2cbus %d\n", *i2cbus);

error:
	globfree(&pglob);

	return ret;
}

static const struct vsrc_ops vcap_gmsl_ops = {
	.set_media_ctrl = vcap_gmsl_ops_set_media_ctrl,
};

struct vlib_vdev *vcap_gmsl_init(const struct matchtable *mte, void *media)
{
	unsigned int i, i2cbus;
	int ret;

	struct vlib_vdev *vd = calloc(1, sizeof(*vd));
	if (!vd) {
		return NULL;
	}

	unsigned int *sensor_id = calloc(1, sizeof(*sensor_id));
	if (!sensor_id) {
		free(vd);
		return NULL;
	}

	vd->vsrc_type = VSRC_TYPE_MEDIA;
	vd->data.media.mdev = media;
	vd->vsrc_class = VLIB_VCLASS_GMSL;
	vd->display_text = "FMC-MULTICAM GMSL";
	vd->entity_name = mte->s;
	vd->ops = &vcap_gmsl_ops;
	vd->flags |= VDEV_CONFIG_TYPE_USER;
	vd->priv = sensor_id;

	vd->data.media.vnode = open(vlib_video_src_mdev2vdev(vd->data.media.mdev), O_RDWR);
	if (vd->data.media.vnode < 0) {
		free(sensor_id);
		free(vd);
		return NULL;
	}

	/* Set active number of lanes */
	gmsl_set_act_lanes(vd, act_lanes);

	/* Get sensor i2c bus */
	ret = ar0231at_get_i2cbus(&i2cbus);
	ASSERT2(ret >= 0, "Failed to detect AR0231 i2c bus.\n");
	snprintf(sensor_entity[0], sizeof(sensor_entity[0]), MEDIA_SENSOR1_ENTITY, i2cbus);
	snprintf(sensor_entity[1], sizeof(sensor_entity[1]), MEDIA_SENSOR2_ENTITY, i2cbus);
	snprintf(sensor_entity[2], sizeof(sensor_entity[2]), MEDIA_SENSOR3_ENTITY, i2cbus);
	snprintf(sensor_entity[3], sizeof(sensor_entity[3]), MEDIA_SENSOR4_ENTITY, i2cbus);
	snprintf(serdes_entity, sizeof(serdes_entity), MEDIA_SERDES_ENTITY, i2cbus);

	/* Set sensor controls */
	for (i = 0; i < GMSL_NUM_SENSORS; i++) {
		ar0231at_set_test_pattern(vd, i, test_pattern[0]);
		ar0231at_set_exposure(vd, i, exposure[0]);
		ar0231at_set_analog_gain(vd, i, analog_gain[0]);
		ar0231at_set_digital_gain(vd, i, digital_gain[0]);
		ar0231at_set_vertical_flip(vd, i, vertical_flip[0]);
		ar0231at_set_horizontal_flip(vd, i, horizontal_flip[0]);
		ar0231at_set_color_gain_red(vd, i, color_gain_red[0]);
		ar0231at_set_color_gain_green(vd, i, color_gain_green[0]);
		ar0231at_set_color_gain_blue(vd, i, color_gain_blue[0]);
	}
	ar0231at_init_test_pattern_names(vd);

	return vd;
}
