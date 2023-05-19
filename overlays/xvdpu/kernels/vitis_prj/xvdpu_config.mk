# (C) Copyright 2020 - 2022 Xilinx, Inc.
# (C) Copyright 2022 - 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************/

# Setting the arch of XVDPU. For more details, please refer to PG389.
# Default parameters:CPB_N = 32, LOAD_PARALLEL_IMG = 2

#PL_FREQ is the frequency of XVDPU's 'm_axi_aclk'. Default is 333M Hz.
PL_FREQ     = 333000000

#Max value of 'UBANK_IMG_N' is 16, and Max value of 'UBANK_WGT_N' is 17.
UBANK_IMG_N = 16
UBANK_WGT_N = 17 
