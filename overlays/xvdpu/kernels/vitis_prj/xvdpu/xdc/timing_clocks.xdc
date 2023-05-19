# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0
#
########################################
# constrains for S_AXI_CLK_INDEPENDENT #
########################################

set clk_rs  [get_clocks -of_objects [get_ports s_axi_aclk]]
set clk_1x  [get_clocks -of_objects [get_ports m_axi_aclk ] ]
expr {($clk_rs!=$clk_1x)?[set_false_path -from $clk_1x  -to $clk_rs]:{}}
expr {($clk_rs!=$clk_1x)?[set_false_path -from $clk_rs  -to $clk_1x]:{}}