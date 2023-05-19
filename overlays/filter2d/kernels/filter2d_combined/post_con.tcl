# (C) Copyright 2020 - 2022 Xilinx, Inc.
# (C) Copyright 2022 - 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0

set_property -dict [list CONFIG.CONNECTIONS {MC_2 { read_bw {1782} write_bw {1782} read_avg_burst {4} write_avg_burst {4}} }] [get_bd_intf_pins /NOC_0/S15_AXI]

#Connect accelerator ports to the performance monitor
connect_bd_intf_net [get_bd_intf_pins axi_perf_mon_0/SLOT_3_AXI] [get_bd_intf_pins NOC_0/S15_AXI]

