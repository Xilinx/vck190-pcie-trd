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

#ifndef VCAP_TPG_H
#define VCAP_TPG_H

#ifdef __cplusplus
extern "C"
{
#endif

#define TPG_BG_PATTERN_CNT 17

struct vlib_vdev;

/* tpg helper functions */
const char *tpg_get_pattern_menu_name(unsigned int idx);
void tpg_set_blanking(const struct vlib_vdev *vd, unsigned int vblank,
		      unsigned int hblank, unsigned int ppc);
void tpg_set_bg_pattern(const struct vlib_vdev *vd, unsigned int bg);
void tpg_set_fg_pattern(const struct vlib_vdev *vd, unsigned int fg);
void tpg_set_box_size(const struct vlib_vdev *vd, unsigned int size);
void tpg_set_box_color(const struct vlib_vdev *vd, unsigned int color);
void tpg_set_box_speed(const struct vlib_vdev *vd, unsigned int speed);
void tpg_set_cross_hair_num_rows(const struct vlib_vdev *vd, unsigned int row);
void tpg_set_cross_hair_num_columns(const struct vlib_vdev *vd, unsigned int column);
void tpg_set_zplate_hor_cntl_start(const struct vlib_vdev *vd, unsigned int hstart);
void tpg_set_zplate_hor_cntl_delta(const struct vlib_vdev *vd, unsigned int hspeed);
void tpg_set_zplate_ver_cntl_start(const struct vlib_vdev *vd, unsigned int vstart);
void tpg_set_zplate_ver_cntl_delta(const struct vlib_vdev *vd, unsigned int vspeed);
#ifdef __cplusplus
}
#endif

#endif /* VCAP_TPG_H */
