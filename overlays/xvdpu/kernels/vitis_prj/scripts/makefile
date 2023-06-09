# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CFG_VH_PATH         = ../xvdpu/vitis_cfg/vitis_cfg.vh
LOAD_PARALLEL_IMG   = 2
SAVE_PARALLEL_IMG   = 2
LOAD_PARALLEL_WGT   = 4
RAM_IMG_ADDR_BW     = 13
RAM_WGT_ADDR_BW     = 13
UBANK_IMG_MRS       = 0
UBANK_WGT_MRS       = 0
UBANK_BIAS          = 1
DBG_ENA             = 0
PROF_ENA            = 0

ifeq  ($(CPB_N), 32)
	BATCH_LIST          = 1 2 3 4 5 6
endif

ifeq  ($(CPB_N), 64)
	BATCH_LIST          = 1 2 3 4 5
endif
	
#Prepare project files: copy xvdpu IP to project folder, generate scripts for special configuration of xvdpu.
.PHONY: all 
all: 
	@valid=0; \
	for p in $(BATCH_LIST); do \
	  if [ "$$p" = "$(BATCH_N)" ]; then \
	    valid=1; \
	    break; \
	  fi \
	done; \
	if [ "$$valid" -ne 1 ]; then \
	  echo 'Invalid batch number $(BATCH_N). Choose one of: $(BATCH_LIST)'; \
	  exit 1; \
	fi; \
	
	#Preparing files for project
	cp -r ../../DPUCVDX8G/xvdpu_ip/aie             ../.	
	cp -r ../../DPUCVDX8G/xvdpu_ip/rtl/hdl         ../xvdpu/.
	cp -r ../../DPUCVDX8G/xvdpu_ip/rtl/vitis_cfg   ../xvdpu/.
	cp -r ../../DPUCVDX8G/xvdpu_ip/rtl/inc         ../xvdpu/.
	cp -r ../../DPUCVDX8G/xvdpu_ip/rtl/ttcl        ../xvdpu/.
	python xvdpu_aie_noc.py $(BATCH_N) $(CPB_N) $(LOAD_PARALLEL_IMG) $(BATCH_SHRWGT_N) $(LOAD_PARALLEL_WGT) && python kernel_xml.py $(BATCH_N) $(LOAD_PARALLEL_IMG) $(CPB_N) $(BATCH_SHRWGT_N) $(LOAD_PARALLEL_WGT)
	
	#Change parameters in 'vitis_cfg.vh'
	sed -i "s/define wrp_CPB_N               .*/define wrp_CPB_N               $(CPB_N)/g" $(CFG_VH_PATH)
	sed -i "s/define wrp_BATCH_N             .*/define wrp_BATCH_N             $(BATCH_N)/g" $(CFG_VH_PATH)
	sed -i "s/define wrp_BATCH_SHRWGT_N      .*/define wrp_BATCH_SHRWGT_N      $(BATCH_SHRWGT_N)/g" $(CFG_VH_PATH)
	sed -i "s/define wrp_UBANK_IMG_N         .*/define wrp_UBANK_IMG_N         $(UBANK_IMG_N)/g" $(CFG_VH_PATH)
	sed -i "s/define wrp_UBANK_WGT_N         .*/define wrp_UBANK_WGT_N         $(UBANK_WGT_N)/g" $(CFG_VH_PATH)
	sed -i "s/define wrp_LOAD_PARALLEL_IMG   .*/define wrp_LOAD_PARALLEL_IMG   $(LOAD_PARALLEL_IMG)/g" $(CFG_VH_PATH)
	sed -i "s/define wrp_SAVE_PARALLEL_IMG   .*/define wrp_SAVE_PARALLEL_IMG   $(SAVE_PARALLEL_IMG)/g" $(CFG_VH_PATH)
	printf "\`define wrp_LOAD_PARALLEL_WGT   $(LOAD_PARALLEL_WGT)\n"  >> $(CFG_VH_PATH)
	printf "\`define wrp_RAM_IMG_ADDR_BW     $(RAM_IMG_ADDR_BW)\n" >> $(CFG_VH_PATH)
	printf "\`define wrp_RAM_WGT_ADDR_BW     $(RAM_WGT_ADDR_BW)\n" >> $(CFG_VH_PATH)
	printf "\`define wrp_UBANK_IMG_MRS       $(UBANK_IMG_MRS)\n" >> $(CFG_VH_PATH)
	printf "\`define wrp_UBANK_WGT_MRS       $(UBANK_WGT_MRS)\n" >> $(CFG_VH_PATH)
	printf "\`define wrp_UBANK_BIAS          $(UBANK_BIAS)\n" >> $(CFG_VH_PATH)
	
ifeq  ($(DBG_ENA), 1)
	printf "\`define wrp_DBG_ENA             1\n"  >> $(CFG_VH_PATH)
endif

ifeq  ($(PROF_ENA), 1)
	printf "\`define wrp_PROF_ENA            1\n"  >> $(CFG_VH_PATH)
endif

ifeq  ($(BATCH_N), 6)
	sed -i "s/define wrp_UBANK_IMG_MRS   .*/define wrp_UBANK_IMG_MRS       1/g"  $(CFG_VH_PATH)
	sed -i "s/define wrp_UBANK_WGT_MRS   .*/define wrp_UBANK_WGT_MRS       1/g"  $(CFG_VH_PATH)
endif
