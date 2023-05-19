FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
KERNEL_FEATURES:append = " bsp.cfg"

SRC_URI += " \
	file://0001-i2c-free_clk.patch \
	file://0003-drm-xlnx_mixer-Dont-enable-primary-plane-by-default.patch \
	file://bsp.cfg \
	file://0001-media-xilinx-Add-isppipeline-driver.patch \
	file://0002-media-xilinx-Add-hdr-extract-driver.patch \
	file://0003-dt-binding-for-hdr-extract-driver.patch \
	file://0004-media-xilinx-Add-hdr-merge-driver.patch \
	file://0005-dt-binding-for-hdr-merge-driver.patch \
	file://0006-imx274-update-for-HDR-mode.patch \
	file://0001-drivers-misc-add-support-for-interrupt-based-PCIe-en.patch \
	file://0002-xilinx_pci_endpoint-Add-resolution-use-case-and-64-b.patch \
	file://0003-Added-ioctl-to-support-different-formats.patch \
	file://0004-added-driver-without-filter-support.patch \
	file://0005-Added-dma-import-and-bufferpool-changes.patch \
	file://0006-Added-VMAP-dma_buf_ops-and-CMA-memory-update.patch \
"
