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
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "helper.h"
#include "mediactl_helper.h"
#include "platform.h"
#include "v4l2_helper.h"

/* set subdevice control */
int v4l2_set_ctrl(const struct vlib_vdev *vsrc, char *name, int id, int value)
{
	int fd, ret;
	char subdev_name[DEV_NAME_LEN];
	struct v4l2_queryctrl query;
	struct v4l2_control ctrl;

	if (!vsrc) {
		return VLIB_ERROR_INVALID_PARAM;
	}

	get_entity_devname(vlib_vdev_get_mdev(vsrc), name, subdev_name);

	fd = open(subdev_name, O_RDWR);
	ASSERT2(fd >= 0, "failed to open %s: %s\n", subdev_name, ERRSTR);

	memset(&query, 0, sizeof(query));
	query.id = id;
	ret = ioctl(fd, VIDIOC_QUERYCTRL, &query);
	ASSERT2(ret >= 0, "VIDIOC_QUERYCTRL failed: %s\n", ERRSTR);

	if (query.flags & V4L2_CTRL_FLAG_DISABLED) {
		vlib_info("V4L2_CID_%d is disabled\n", id);
	} else {
		memset(&ctrl, 0, sizeof(ctrl));
		ctrl.id = query.id;
		ctrl.value = value;
		ret = ioctl(fd, VIDIOC_S_CTRL, &ctrl);
		ASSERT2(ret >= 0, "VIDIOC_S_CTRL failed: %s\n", ERRSTR);
	}

	close(fd);
	return VLIB_SUCCESS;
}
