#
# This is the vck190-pcie-trd recipe
#
#

SUMMARY = "VCK190 PCIe TRD Jupyter notebooks"
SECTION = "PETALINUX/apps"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=972c7f0cb5739eb512a94e5c9468384f"

S = "${WORKDIR}"

SRC_URI = " \
	file://cmake \
	file://vvas-accel-sw-libs \
	file://notebooks \
	file://CMakeLists.txt \
	file://LICENSE \
	"

inherit cmake

DEPENDS += " \
	vvas-utils \
	jansson \
	opencv \
	"

RDEPENDS:${PN} += " \
	gstreamer1.0-plugins-bad \
	gstreamer1.0-plugins-good \
	gstreamer1.0-python \
	vvas-accel-libs \
	vvas-gst \
	python3-notebook \
	python3-opencv \
	python3-pydot \
	"

FILES:${PN} += " \
	${libdir} \
	${datadir} \
	"
