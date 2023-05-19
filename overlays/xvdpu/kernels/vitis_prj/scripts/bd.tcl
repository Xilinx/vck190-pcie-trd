# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0
#
proc init {cellpath otherInfo} {
  set ip [get_bd_cells $cellpath]

}

proc post_config_ip {cellpath otherInfo} {
  set ip [get_bd_cells $cellpath]
  if { [get_property CONFIG.S_AXI_CLK_INDEPENDENT $ip] == {1} } {
    bip_set_clk_busif "$ip/m_axi_aclk" {S_AXI_CONTROL} {rmv}
  } else {
    bip_set_clk_busif "$ip/m_axi_aclk" {S_AXI_CONTROL} {add}
  }
  bip_set_time_stamp $ip

}

proc pre_propagate {cellpath otherInfo} {
  set ip [get_bd_cells $cellpath]
}

proc propagate {cellpath otherInfo} {
  set ip [get_bd_cells $cellpath]

  set_property "CONFIG.M_AXI_FREQ_MHZ"    [expr round(([get_property CONFIG.FREQ_HZ [get_bd_pins "$ip/m_axi_aclk"]]+0.0)/1000000)]  $ip
  if { [get_property CONFIG.S_AXI_CLK_INDEPENDENT $ip] == {1} } {
    set_property "CONFIG.S_AXI_FREQ_MHZ"  [expr round(([get_property CONFIG.FREQ_HZ [get_bd_pins "$ip/s_axi_aclk"]]+0.0)/1000000)]  $ip
  } else {
    set_property "CONFIG.S_AXI_FREQ_MHZ"  [expr round(([get_property CONFIG.FREQ_HZ [get_bd_pins "$ip/m_axi_aclk"]]+0.0)/1000000)]  $ip
  }

}

proc post_propagate {cellpath otherInfo} {
  set ip [get_bd_cells $cellpath]

  bip_set_time_stamp $ip

}

proc bip_set_clk_busif { pin_clk busif_name {mode add} } {
  set busifs      [get_property CONFIG.ASSOCIATED_BUSIF [get_bd_pins $pin_clk]]
  set busif_subs  $busif_name
  if { [set has_busif [string match *$busif_subs* $busifs]] == {1} } {
    set busif_subs $busif_name:
    if { [string match *$busif_subs* $busifs] != {1} } {
      set busif_subs :$busif_name
      if { [string match *$busif_subs* $busifs] != {1} } {
        set busif_subs $busif_name
      }
    }
  }
  switch $mode {
    add     { if { $has_busif == {0} } { set busifs $busifs:$busif_name                         } }
    rmv     { if { $has_busif == {1} } { set busifs [string map [list $busif_subs {}] $busifs]  } }
    default {}
  }
  set_property CONFIG.ASSOCIATED_BUSIF $busifs [get_bd_pins $pin_clk]
}

proc bip_set_time_stamp {ip} {
  set ena [get_property "CONFIG.TIMESTAMP_ENA" $ip]
  if { $ena > {0} } {
    set_property "CONFIG.TIME_YEAR"     [expr [scan [clock format [clock seconds] -format %Y] %d] -2000 ] $ip
    set_property "CONFIG.TIME_MONTH"    [expr [scan [clock format [clock seconds] -format %m] %d]       ] $ip
    set_property "CONFIG.TIME_DAY"      [expr [scan [clock format [clock seconds] -format %e] %d]       ] $ip
    set_property "CONFIG.TIME_HOUR"     [expr [scan [clock format [clock seconds] -format %k] %d]       ] $ip
    set_property "CONFIG.TIME_QUARTER"  [expr [scan [clock format [clock seconds] -format %M] %d] /15   ] $ip
  }
}

#
