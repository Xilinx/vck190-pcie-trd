# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************/

import sys
import os

Batch_N    = int(sys.argv[1])
CPB        = int(sys.argv[2])
LOAD_I_P   = int(sys.argv[3])
BAT_SHRWGT = int(sys.argv[4])
LOAD_W_P   = int(sys.argv[5])

cu_num     = 1

WGTBC_N    = int((Batch_N+BAT_SHRWGT-1)/BAT_SHRWGT)
IMG_ports  = Batch_N*LOAD_I_P

if CPB==16:
    ifm_number = 8*Batch_N 
    ofm_number = 8*Batch_N
    wgt_number = 4*WGTBC_N
elif CPB==32:
    ifm_number = 8*Batch_N    
    ofm_number = 16*Batch_N
    wgt_number = 8*WGTBC_N
elif CPB==64:
    ifm_number = 8*Batch_N
    ofm_number = 32*Batch_N
    wgt_number = 16*WGTBC_N
else:
    pass
 
 
result  = "[connectivity]\n"
result += "nk=DPUCVDX8G:1:DPUCVDX8G_1\n"
result += "nk=image_processing:1:image_processing_1\n"
###########AXI-stream connection between XVDPU and AIE#################################
    
def genOFM(cu, ofm_num):
    char = ""
    for i in range (ofm_num):
        sc_num    = str(i).rjust(2,'0')
        sc_num_cu = str(i+cu*ofm_num).rjust(2,'0')
        char     += "stream_connect=ai_engine_0.M" + sc_num_cu + "_AXIS:DPUCVDX8G_" + str(cu+1) + ".S" + sc_num + "_OFM_AXIS\n"
    return char

def genIFM(cu, ifm_num):
    char = ""
    for i in range (ifm_num):
        sc_num    = str(i).rjust(2,'0')
        sc_num_cu = str(i+cu*ifm_num).rjust(2,'0')
        char     += "stream_connect=DPUCVDX8G_" + str(cu+1) +".M" + sc_num + "_IFM_AXIS:ai_engine_0.S" + sc_num_cu + "_AXIS\n"
    return char

def genWGT(cu, wgt_num, ifm_total):
    char = ""
    for i in range (wgt_num):
        sc_num    = str(i).rjust(2,'0')
        sc_num_cu = str(i+cu*wgt_num+ifm_total).rjust(2,'0')
        char     += "stream_connect=DPUCVDX8G_" + str(cu+1) +".M" + sc_num + "_WGT_AXIS:ai_engine_0.S" + sc_num_cu + "_AXIS\n"
    return char

for i in range (cu_num):
    result += genOFM(i,ofm_number)

for i in range (cu_num):
    result += genIFM(i,ifm_number)

for i in range (cu_num):
    result += genWGT(i,wgt_number,ifm_number*cu_num)
    
###########sptag section for XVDPU and NOC########################################
global S_AXI_N 
S_AXI_N = 15

#Name of sptag is: "NOC_S" + str(S_AXI_N)
def genSP(cu, axi_name, ports_num):
    char = ""
    global S_AXI_N 
    for i in range (ports_num):
        char += "sp=DPUCVDX8G_" + str(cu+1) + ".M" + str(i).rjust(2,'0') + "_" + axi_name + "_AXI:" + "NOC_S" + str(S_AXI_N) + "\n"
        S_AXI_N = S_AXI_N + 1
    return char   
       
for i in range (cu_num):
    result += genSP(i, "INSTR", 1)
    result += genSP(i, "BIAS", 1)
    result += genSP(i, "WGT", LOAD_W_P)
    result += genSP(i, "IMG", IMG_ports)
    
# sptag for v_multi_scaler. sptag name is ' "NOC_S" + number_axi '.
for i in range (1):    
    number_axi = str(S_AXI_N)
    sptag_name = "NOC_S" + number_axi
    result += "sp=image_processing_1.m_axi_mm_video"  + ":" + sptag_name + "\n"
    S_AXI_N = S_AXI_N + 1
 
    
file_name="xvdpu_aie_noc" + ".cfg"
new_file = open(file_name, "w+")
new_file.write(result)
new_file.close()
