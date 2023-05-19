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

#ifndef VLIB_PLATFORM_H
#define VLIB_PLATFORM_H

#ifdef __cplusplus
extern "C"
{
#endif

#define HRES_2160P      3840
#define VRES_2160P      2160
#define HRES_1080P      1920
#define VRES_1080P      1080
#define HRES_720P       1280
#define VRES_720P        720

/* Platform specific configuration */
#if defined(PLATFORM_ZCU102) || defined(PLATFORM_ZC1751_DC1)
#define MAX_HEIGHT           VRES_2160P
#define MAX_WIDTH            HRES_2160P
#define MAX_STRIDE           MAX_WIDTH
#define DRM_MODULE           DRM_MODULE_XILINX
#define DRM_ALPHA_PROP       "alpha"
#define DRM_MAX_ALPHA        255
#define GPIO_PS_BASE_OFFSET  338
#elif defined(PLATFORM_ZC70X)
#define MAX_HEIGHT           VRES_1080P
#define MAX_WIDTH            HRES_1080P
#define MAX_STRIDE           2048 // Xylon has fixed stride (has to be power of 2 and greater or equal than MAX_WIDTH)
#define DRM_MODULE           DRM_MODULE_XYLON
#define DRM_ALPHA_PROP       "transparency"
#define DRM_MAX_ALPHA        255
#define GPIO_PS_BASE_OFFSET  906
#endif

#ifdef __cplusplus
}
#endif

#endif /* VLIB_PLATFORM_H */
