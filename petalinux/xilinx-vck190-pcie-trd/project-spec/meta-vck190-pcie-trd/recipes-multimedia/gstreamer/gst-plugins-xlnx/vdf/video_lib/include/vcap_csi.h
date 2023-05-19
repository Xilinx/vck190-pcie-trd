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

#ifndef VCAP_CSI_H
#define VCAP_CSI_H

#ifdef __cplusplus
extern "C"
{
#endif

#define IMX274_TEST_PATTERN_CNT	13

struct vlib_vdev;

void gamma_set_blue_correction(const struct vlib_vdev *vd, unsigned int blue);
void gamma_set_green_correction(const struct vlib_vdev *vd, unsigned int green);
void gamma_set_red_correction(const struct vlib_vdev *vd, unsigned int red);

void csc_set_brightness(const struct vlib_vdev *vd, unsigned int bright);
void csc_set_contrast(const struct vlib_vdev *vd, unsigned int cont);
void csc_set_blue_gain(const struct vlib_vdev *vd, unsigned int blue);
void csc_set_green_gain(const struct vlib_vdev *vd, unsigned int green);
void csc_set_red_gain(const struct vlib_vdev *vd, unsigned int red);

void imx274_set_exposure(const struct vlib_vdev *vd, unsigned int exp);
void imx274_set_gain(const struct vlib_vdev *vd, unsigned int gn);
void imx274_set_vertical_flip(const struct vlib_vdev *vd, unsigned int vflip);
void imx274_set_test_pattern(const struct vlib_vdev *vd, unsigned int tp);
const char *imx274_get_test_pattern_name(unsigned int idx);

#ifdef __cplusplus
}
#endif

#endif /* VCAP_CSI_H */
