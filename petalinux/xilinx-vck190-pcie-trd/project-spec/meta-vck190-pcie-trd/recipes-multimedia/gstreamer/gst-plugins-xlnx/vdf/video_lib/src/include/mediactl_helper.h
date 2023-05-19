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

#ifndef MEDIA_CTL_H
#define MEDIA_CTL_H

struct media_device;
struct media_device_info;
struct vlib_vdev;
struct v4l2_dv_timings;

/* Display media device info */
void print_media_info(const struct media_device_info *info);
/* Retrieve the detected digital video timings */
int query_entity_dv_timings(const struct vlib_vdev *vdev, char *name,
			    unsigned int padn, struct v4l2_dv_timings *timings);
/* Returns the full path and name to the device node */
int get_entity_devname(struct media_device *media, char *name, char *subdev_name);
/* Set media format string */
void media_set_fmt_str(char *set_fmt, char *entity, unsigned int pad,
		       const char *fmt, unsigned int width, unsigned int height);
/* Set media pad string */
void media_set_pad_str(char *set_fmt, char *entity, unsigned int pad);

#endif /* MEDIA_CTL_H */
