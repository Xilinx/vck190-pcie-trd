#
# This is the gst-plugins-xlnx recipe
#
#

SUMMARY = "gst plugins for Xilinx video source and sink"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${WORKDIR}/src/gstxlnxvideosrc/gstxlnxvideosrc.c;beginline=1;endline=24;md5=091ea5d307cba07f2969994cdbb8f2f3"

DEPENDS = "libdrm v4l-utils glib-2.0 gstreamer1.0 gstreamer1.0-plugins-base"

SRC_URI = " \
	file://CMakeLists.txt \
	file://src \
	file://vdf \
	"

S = "${WORKDIR}"

inherit pkgconfig cmake

FILES:${PN} += " \
        ${libdir}/gstreamer-1.0/*.so \
        "
