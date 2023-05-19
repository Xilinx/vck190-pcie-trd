# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0


############################## Help ##############################
.PHONY: help
help:
	@echo "Makefile Usage:"
	@echo ""
	@echo "  make all PLATFORM=<VCK190 platform> BATCH_N=<>"
	@echo "      Command to generate all design"
	@echo "      Support platform 'vck190_mipiRxSingle_hdmiTx'"
	@echo ""
	@echo "  make clean "
	@echo "      Command to remove IP files from vitis_prj folder and all the generated files"
	@echo ""
############################ Project Variables #####################
#Included files:
include xvdpu_config.mk

ABS_PATH                := $(shell pwd -P)
$(eval FATHER_PATH      := $(realpath $(ABS_PATH)/..))
# PLATFORM                 =
DEVICE                  ?= vck190
XCL_EMULATION_MODE      ?= hw
unexport XCL_EMULATION_MODE
#Build directory
BUILD_DIR                = $(XCL_EMULATION_MODE)
#Default v++ compiler flags
VXXFLAGS                += -t $(XCL_EMULATION_MODE) --platform $(PLATFORM) --save-temps --temp_dir $(BUILD_DIR)/binary_container_1 -g
VXXFLAGS                += --xp vivado_prop:run.impl_1.STEPS.PLACE_DESIGN.TCL.PRE=$(ABS_PATH)/scripts/pre_place.tcl
VXXFLAGS                += --xp vivado_prop:run.impl_1.STEPS.PLACE_DESIGN.TCL.POST=$(ABS_PATH)/scripts/post_place.tcl
VXXFLAGS                += --xp param:compiler.userPostSysLinkOverlayTcl=$(ABS_PATH)/scripts/post_linker.tcl
VXXFLAGS                += --xp vivado_prop:run.impl_1.GEN_FULL_BITSTREAM=1
VXXFLAGS                += --log_dir $(BUILD_DIR)/binary_container_1/logs
VXXFLAGS                += --report_dir $(BUILD_DIR)/binary_container_1/reports
VXXFLAGS                += --xp vivado_prop:run.impl_1.STEPS.POST_ROUTE_PHYS_OPT_DESIGN.IS_ENABLED=false

VXX                      = $(XILINX_VITIS)/bin/v++
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Build products
AIE_OBJECTS              = $(ABS_PATH)/aie/ip/libadf.a
BINARY_CONTAINERS       += $(BUILD_DIR)/binary_container_1.xsa
BUILD_SUBDIRS           += $(BUILD_DIR)/binary_container_1
BINARY_CONTAINER_1_OBJS += $(BUILD_DIR)/binary_container_1/DPUCVDX8G.xo $(BUILD_DIR)/binary_container_1/libadf.a image_processing.xo
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Package RTL kernel
VIVADO_ROOT             := $(XILINX_VIVADO)
VIVADO                  := $(VIVADO_ROOT)/bin/vivado
xvdpu_TCL                = $(ABS_PATH)/scripts/gen_xvdpu_xo.tcl
DPU_KERN_NAME            = DPUCVDX8G
BATCH_SHRWGT_N           = 4
AIE_PL_FREQ              = 250
CPB_N                    = 32
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
.PHONY: all
all:  files $(AIE_OBJECTS) xo_image_processing $(BINARY_CONTAINERS) $(BUILD_DIR)/dpu.xclbin
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Prepare files for special configuration
files:
	@echo "INFO:- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
	@echo "INFO:Preparing files for project"
	${MAKE} -C scripts all CPB_N=$(CPB_N) BATCH_N=$(BATCH_N) UBANK_IMG_N=$(UBANK_IMG_N) UBANK_WGT_N=$(UBANK_WGT_N) BATCH_SHRWGT_N=$(BATCH_SHRWGT_N)

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Compile AIE IP as 'libadf.a'
.PHONY: aie
aie: $(ABS_PATH)/aie/ip/libadf.a

$(ABS_PATH)/aie/ip/libadf.a:
	@echo "INFO:- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
	@echo "INFO:Buiding $(notdir $@)..."
	@XILINX_VITIS_AIETOOLS=$(XILINX_VITIS)/aietools source $(XILINX_VITIS)/settings64.sh && ${MAKE} --no-print-directory -C aie ip CPB=$(CPB_N) BAT_NUM=$(BATCH_N) vver=221 pl_freq=$(AIE_PL_FREQ) aie_activations_en=1 bat_sharewgt=$(BATCH_SHRWGT_N)
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Copy AIE's libadf.a to folder $(BUILD_SUBDIRS)
$(BUILD_DIR)/binary_container_1/libadf.a:
	@echo "INFO:- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
	@echo "INFO:Copying $(notdir $@)..."
	@mkdir -p $(@D)
	@cp aie/ip/libadf.a "$@"
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Compile multiscaler kernel as XO file
VVAS_CONFIG    = vvas/vvas-accel-hw/image_processing/image_processing_config.h
VVAS_MAKEFILE  = vvas/vvas-accel-hw/image_processing/Makefile

.PHONY: xo_image_processing
xo_image_processing: image_processing.xo
image_processing.xo:
	@echo "INFO:- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
	@echo "INFO:Building $(notdir $@)"
	-@rm -rf image_processing.xo
	sed -i "s/#define HSC_SAMPLES_PER_CLOCK       2/#define HSC_SAMPLES_PER_CLOCK       4/g" $(VVAS_CONFIG)
	sed -i "s/#define HSC_MAX_WIDTH               1920/#define HSC_MAX_WIDTH               3840/g" $(VVAS_CONFIG)
	sed -i "s/#define HSC_MAX_HEIGHT              1080/#define HSC_MAX_HEIGHT              2160/g" $(VVAS_CONFIG)
	sed -i "s/PLATFORM_FILE/PLATFORM/g"           $(VVAS_MAKEFILE)
	$(MAKE) -C vvas/vvas-accel-hw/image_processing all PLATFORM=$(PLATFORM)
	@cp vvas/vvas-accel-hw/image_processing/xo/image_processing.xo image_processing.xo
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Compile xvdpu kernel as XO file
.PHONY: xo_xvdpu
xo_xvdpu: $(BUILD_DIR)/binary_container_1/DPUCVDX8G.xo

$(BUILD_DIR)/binary_container_1/DPUCVDX8G.xo:
	@echo "INFO:- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
	@echo "INFO:Building $(notdir $@) for $(BUILD_DIR)..."
	@mkdir -p $(@D)
	-@rm -rf $@
	$(VIVADO) -mode batch -source $(xvdpu_TCL) -tclargs $@ $(DPU_KERN_NAME) $(XCL_EMULATION_MODE) $(DEVICE)
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Link kernel and AIE graph with Platform
$(BUILD_DIR)/binary_container_1.xsa: $(BINARY_CONTAINER_1_OBJS)
	@echo "INFO:- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
	@echo "INFO:Building $(notdir $@) for $(BUILD_DIR)..."
	@mkdir -p $(@D)
	@$(VXX) $(VXXFLAGS) -l --config scripts/system.cfg --config scripts/xvdpu_aie_noc.cfg \
	  --clock.freqHz $(PL_FREQ):DPUCVDX8G_1.m_axi_aclk \
	  --clock.freqHz $(PL_FREQ):ai_engine_0.aclk0 \
	  --clock.freqHz 150000000:DPUCVDX8G_1.s_axi_aclk \
	  --clock.freqHz 150000000:image_processing_1.ap_clk \
	  -o "$@" $(BINARY_CONTAINER_1_OBJS)
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Package design
.PHONY: package
package: $(BUILD_DIR)/dpu.xclbin
$(BUILD_DIR)/dpu.xclbin: $(BUILD_DIR)/binary_container_1.xsa $(BUILD_DIR)/binary_container_1/libadf.a
	@echo "INFO:- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
	@echo "INFO:Packaging $(notdir $@) for $(BUILD_DIR)..."
	-@rm -rf package_out
	@cd $(BUILD_DIR); $(VXX) -t $(XCL_EMULATION_MODE) --platform $(PLATFORM)  --save-temps --temp_dir binary_container_1 -p binary_container_1.xsa binary_container_1/libadf.a -o "$@" \
	  --package.out_dir ../package_out \
	  --package.boot_mode sd \
	  --package.no_image
	cp  $(ABS_PATH)/hw/binary_container_1/link/vivado/vpl/prj/prj.gen/sources_1/bd/*/ip/*_DPUCVDX8G_1_0/arch.json $(ABS_PATH)/package_out/sd_card
	@cp package_out/sd_card/BOOT.BIN vpl_gen_fixed.pdi
	@cp package_out/sd_card/dpu.xclbin binary_container_1.xclbin
	@cp $(BUILD_DIR)/binary_container_1.xsa binary_container_1.xsa
	@zip -u binary_container_1.xsa vpl_gen_fixed.pdi
	@zip -u binary_container_1.xsa binary_container_1.xclbin
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Clean build products
.PHONY: clean
clean:
	-@rm -rf .Xil vivado* *.log .ipcache sample*.ini packaged_kernel_*_hw_* tmp_kernel_pack_*_hw_*
	-@rm -rf scripts/kernel.xml scripts/*xvdpu_aie_noc.cfg xvdpu/hdl xvdpu/inc xvdpu/vitis_cfg xvdpu/ttcl
	-@rm -rf hw package_out *.xsa *.pdi *.xclbin image_processing.xo
	$(MAKE) -C vvas/vvas-accel-hw/image_processing/ clean
	-@rm -rf aie
