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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mediactl/mediactl.h>
#include <mediactl/v4l2subdev.h>
#include <unistd.h>

#include <helper.h>
#include <mediactl_helper.h>
#include <v4l2_helper.h>
#include <vcap_hdmi_int.h>

#define MEDIA_G_DVTIMINGS_RETRY_CNT 10
#define MEDIA_G_DVTIMINGS_RETRY_DLY_USEC 100000

#if defined(PLATFORM_ZCU102)
#define MEDIA_ADV7611_ENTITY	"adv7611 25-004c"
#define MEDIA_HDMI_RXSS_ENTITY	"a1000000.hdmi_rxss"
#define VCAP_HDMI_HAS_SCALER 1
#elif defined (PLATFORM_ZC1751_DC1) || defined(PLATFORM_ZC70X)
#define MEDIA_ADV7611_ENTITY	"adv7611 12-004c"
#define MEDIA_HDMI_RXSS_ENTITY	" "
#define VCAP_HDMI_HAS_SCALER 0
#endif

#ifdef HDMI_ADV7611
#define MEDIA_HDMI_ENTITY	MEDIA_ADV7611_ENTITY
#define MEDIA_HDMI_FMT_OUT	"UYVY"
#define MEDIA_HDMI_PAD		1
#else /* HDMI_RXSS */
#define MEDIA_HDMI_ENTITY	MEDIA_HDMI_RXSS_ENTITY
#define MEDIA_HDMI_PAD		0
#endif

#if defined(PLATFORM_ZCU102) || defined (PLATFORM_ZC1751_DC1)
#define MEDIA_SCALER_ENTITY	"b0100000.scaler"
#elif defined(PLATFORM_ZC70X)
#define MEDIA_SCALER_ENTITY	" "
#endif

struct vcap_hdmi_data {
	size_t in_width;
	size_t in_height;
	unsigned int flags;
};
#define VCAP_HDMI_FLAG_HAS_SCALER	BIT(0)

static int vcap_hdmi_has_scaler(const struct vlib_vdev *vd)
{
	const struct vcap_hdmi_data *data = vd->priv;
	ASSERT2(data, "no private data found\n");

	return !!(data->flags & VCAP_HDMI_HAS_SCALER);
}

static int vcap_hdmi_ops_set_media_ctrl(struct video_pipeline *video_setup,
					const struct vlib_vdev *vdev)
{
	int ret = VLIB_SUCCESS;
	struct media_pad *pad;
	struct v4l2_dv_timings timings;
	int retry_cnt = MEDIA_G_DVTIMINGS_RETRY_CNT;
	char fmt_str[100];
	struct media_device *media = vlib_vdev_get_mdev(vdev);
	struct v4l2_mbus_framefmt format;
	const char* fmt_code;
	struct vcap_hdmi_data *data = vdev->priv;

	ASSERT2(data, "no private data found\n");

	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	ASSERT2(ret >= 0, "failed to enumerate %s\n", vdev->display_text);

#ifdef VLIB_LOG_LEVEL_DEBUG
	const struct media_device_info *info = media_get_info(media);
	print_media_info(info);
#endif

	/* Get HDMI Rx pad */
	memset(fmt_str, 0, sizeof(fmt_str));
	media_set_pad_str(fmt_str, MEDIA_HDMI_ENTITY, MEDIA_HDMI_PAD);
	pad = media_parse_pad(media, fmt_str, NULL);
	ASSERT2(pad, "Pad '%s' not found\n", fmt_str);

	/* Repeat query dv_timings as occasionally the reported timings are incorrect */
	do {
		retry_query_timing:
		ret = v4l2_subdev_query_dv_timings(pad->entity, &timings);
		if (ret < 0 && retry_cnt--) {
			/* Delay dv_timings query in-case of failure */
			usleep(MEDIA_G_DVTIMINGS_RETRY_DLY_USEC);
			goto retry_query_timing;
		}
	} while (!vcap_hdmi_has_scaler(vdev) &&
		 (timings.bt.width != video_setup->w ||
		 timings.bt.height != video_setup->h || !retry_cnt--));
	ASSERT2(!(ret), "Failed to query DV timings: %s\n", strerror(-ret));
	ASSERT2(!(retry_cnt < 0), "Incorrect HDMI Rx DV timings: %dx%d\n",
		timings.bt.width, timings.bt.height);

	if (retry_cnt < MEDIA_G_DVTIMINGS_RETRY_CNT)
		vlib_dbg("Link to HDMI source recovered (required retries: %d)\n", MEDIA_G_DVTIMINGS_RETRY_CNT-retry_cnt);

#ifdef HDMI_ADV7611
	/* Set HDMI Rx DV timing */
	ret = v4l2_subdev_set_dv_timings(pad->entity, &timings);
	ASSERT2(!(ret < 0), "Failed to set DV timings: %s\n", strerror(-ret));

	/* Set HDMI Rx resolution */
	memset(fmt_str, 0, sizeof(fmt_str));
	media_set_fmt_str(fmt_str, MEDIA_HDMI_ENTITY, 1, MEDIA_HDMI_FMT_OUT,
			  data->in_width, data->in_height);
	ret = v4l2_subdev_parse_setup_formats(media, fmt_str);
	ASSERT2(!(ret), "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);
#endif

	/* Retrieve HDMI Rx pad format */
	ret = v4l2_subdev_get_format(pad->entity, &format, MEDIA_HDMI_PAD,
				     V4L2_SUBDEV_FORMAT_ACTIVE);
	ASSERT2(!(ret), "Failed to get HDMI Rx pad format: %s\n",
		strerror(-ret));
	fmt_code = v4l2_subdev_pixelcode_to_string(format.code);
	vlib_dbg("HDMI Rx source pad format: %s, %ux%u\n", fmt_code,
		 format.width, format.height);

	/* Set Scaler resolution */
	if (vcap_hdmi_has_scaler(vdev)) {
		memset(fmt_str, 0, sizeof(fmt_str));
		media_set_fmt_str(fmt_str, MEDIA_SCALER_ENTITY, 0, fmt_code,
				  data->in_width, data->in_height);
		ret = v4l2_subdev_parse_setup_formats(media, fmt_str);
		ASSERT2(!(ret), "Unable to setup formats: %s (%d)\n",
			strerror(-ret), -ret);

		memset(fmt_str, 0, sizeof(fmt_str));
		media_set_fmt_str(fmt_str, MEDIA_SCALER_ENTITY, 1,
				  vlib_fourcc2mbus(video_setup->in_fourcc),
				  video_setup->w, video_setup->h);
		ret = v4l2_subdev_parse_setup_formats(media, fmt_str);
		ASSERT2(!(ret), "Unable to setup formats: %s (%d)\n",
			strerror(-ret), -ret);
	}

	return ret;
}

static int vcap_hdmi_ops_change_mode(struct video_pipeline *video_setup,
				     struct vlib_config *config)
{
	int ret;
	struct v4l2_dv_timings dv_timings;
	const struct vlib_vdev *vdev = vlib_video_src_get(config->vsrc);
	struct vcap_hdmi_data *data = vdev->priv;

	ASSERT2(vdev, "no HDMI device\n");
	ASSERT2(data, "no private data found\n");

	/* Query input resolution */
	ret = query_entity_dv_timings(vdev, MEDIA_HDMI_ENTITY, MEDIA_HDMI_PAD,
				      &dv_timings);
	if (ret) {
		VLIB_REPORT_ERR("Query DV timings failed: %s",
				strerror(errno));
		return VLIB_ERROR_CAPTURE;
	}

	data->in_width = dv_timings.bt.width;
	data->in_height = dv_timings.bt.height;

	if (!vcap_hdmi_has_scaler(vdev) && (data->in_width != video_setup->w ||
	    data->in_height != video_setup->h)) {
		VLIB_REPORT_ERR("HDMI input resolution '%zux%zu' does not match plane or display '%ux%u'",
			data->in_width, data->in_height, video_setup->w,
			video_setup->h);
		vlib_info("Continue with previous mode\n");
		return VLIB_ERROR_CAPTURE;
	}

	return 0;
}

static const struct vsrc_ops vcap_hdmi_ops = {
	.change_mode = vcap_hdmi_ops_change_mode,
	.set_media_ctrl = vcap_hdmi_ops_set_media_ctrl,
};

struct vlib_vdev *vcap_hdmi_init(const struct matchtable *mte, void *media)
{
	struct vlib_vdev *vd = calloc(1, sizeof(*vd));
	if (!vd) {
		return NULL;
	}

	struct vcap_hdmi_data *data = calloc(1, sizeof(*data));
	if (!data) {
		free(vd);
		return NULL;
	}

	if (VCAP_HDMI_HAS_SCALER) {
		data->flags |= VCAP_HDMI_FLAG_HAS_SCALER;
	}

	vd->vsrc_type = VSRC_TYPE_MEDIA;
	vd->data.media.mdev = media;
	vd->vsrc_class = VLIB_VCLASS_HDMII;
	vd->display_text = "HDMI Input";
	vd->entity_name = mte->s;
	vd->ops = &vcap_hdmi_ops;
	vd->priv = data;

	const char *fn = vlib_video_src_mdev2vdev(vd->data.media.mdev);
	vd->data.media.vnode = open(fn, O_RDWR);
	if (vd->data.media.vnode < 0) {
		free(data);
		free(vd);
		return NULL;
	}

	return vd;
}
