# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0

PLATFORM ?=
PFM_LIST = vck190_mipiRxSingle_hdmiTx
PFM = $(shell basename $(PLATFORM) .xpfm)
$(info $(PLATFORM))
$(info $(PFM))
BATCH_N_LIST = 1
# hardcode value for now
BATCH_N = 1
DPU_PACKAGE = DPUCVDX8G.tar.gz

.PHONY: all
all: $(DPU_PACKAGE)
	@valid=0; \
	for p in $(PFM_LIST); do \
	  if [ "$$p" = "$(PFM)" ]; then \
	    valid=1; \
	    break; \
	  fi \
	done; \
	if [ "$$valid" -ne 1 ]; then \
	  echo 'Invalid platform $(PFM). Choose one of: $(PFM_LIST)'; \
	  exit 1; \
	fi; \
	$(MAKE) -C vitis_prj all PLATFORM=$(PLATFORM) BATCH_N=$(BATCH_N) -B
	-@cp vitis_prj/binary_container_1.xclbin binary_container_1.xclbin
	-@cp vitis_prj/binary_container_1.xsa binary_container_1.xsa
	-@cp vitis_prj/package_out/sd_card/arch.json arch.json

$(DPU_PACKAGE):
	wget -O $(DPU_PACKAGE) https://www.xilinx.com/bin/public/openDownload?filename=DPUCVDX8G.tar.gz
	@tar -xvf $(DPU_PACKAGE)

.PHONY: clean
clean:
	$(MAKE) -C vitis_prj clean
	-@rm -rf *.xsa *.xclbin
