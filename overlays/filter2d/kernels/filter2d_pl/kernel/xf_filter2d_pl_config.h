// (C) Copyright 2023 Advanced Micro Devices, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef _XF_FILTER2D_PL_CONFIG_H_
#define _XF_FILTER2D_PL_CONFIG_H_

#include "common/xf_common.hpp"
#include "common/xf_utility.hpp"
#include "imgproc/xf_custom_convolution.hpp"

#define WIDTH 3840
#define HEIGHT 2160
#ifndef PPC
#define PPC XF_NPPC8
#endif

const uint32_t pCOLS = WIDTH / PPC;
const uint32_t pCOLSD = WIDTH * 2 / PPC;

extern "C" void filter2d_pl_accel(XF_TNAME(XF_16UC1, PPC) * frm_in,
                       XF_TNAME(XF_16UC1, PPC) * frm_out,
                       int16_t* kernel,
                       uint32_t rows,
                       uint32_t cols,
                       uint32_t fourcc_in,
                       uint32_t fourcc_out);

#endif // end of  _XF_FILTER2D_PL_CONFIG_H_
