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

#include <v4l2_helper.h>
#include <video_int.h>
#include <sys/ioctl.h>

#define VIVID_CTRL_TEST_PATTERN	0xf0f000
#define VIVID_CTRL_OSD_TXT_MODE	0xf0f001
#define VIVID_CTRL_HOR_MOVEMENT	0xf0f002
#define VIVID_CTRL_VER_MOVEMENT	0xf0f003

struct vlib_vdev *vcap_vivid_init(const struct matchtable *mte, void *data)
{
	int ret;
	int fd = (uintptr_t)data;

	struct v4l2_capability vcap;
	ret = ioctl(fd, VIDIOC_QUERYCAP, &vcap);
	if (ret) {
		return NULL;
	}

	if (!(vcap.capabilities & V4L2_CAP_DEVICE_CAPS)) {
		return NULL;
	}

	if (!(vcap.device_caps & V4L2_CAP_VIDEO_CAPTURE)) {
		return NULL;
	}

	if (!(vcap.device_caps & V4L2_CAP_STREAMING)) {
		return NULL;
	}

	struct vlib_vdev *vd = calloc(1, sizeof(*vd));
	if (!vd) {
		return NULL;
	}

	vd->vsrc_type = VSRC_TYPE_V4L2;
	vd->data.v4l2.vnode = fd;
	vd->vsrc_class = VLIB_VCLASS_VIVID;
	vd->display_text = "Virtual Video Device";
	vd->entity_name = mte->s;

	/* set default test pattern */
	struct v4l2_ext_control ctrl[4];
	memset(&ctrl, 0, sizeof(ctrl));
	/* test pattern: color squares */
	ctrl[0].id = VIVID_CTRL_TEST_PATTERN;
	ctrl[0].value = 4;
	/* horizontal movement: fast right */
	ctrl[1].id = VIVID_CTRL_HOR_MOVEMENT;
	ctrl[1].value = 6;
	/* vertical movement: fast down */
	ctrl[2].id = VIVID_CTRL_VER_MOVEMENT;
	ctrl[2].value = 6;
	/* osd text mode: Disable text menu as videoinfo panel overlays it */
	ctrl[3].id = VIVID_CTRL_OSD_TXT_MODE;
	ctrl[3].value = 2;

	struct v4l2_ext_controls ctrls;
	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
	ctrls.which = V4L2_CTRL_WHICH_CUR_VAL;
	ctrls.count = ARRAY_SIZE(ctrl);
	ctrls.controls = ctrl;

	ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, &ctrls);
	ASSERT2(!ret, "failed to set vivid controls: %s", strerror(errno));

	return vd;
}
