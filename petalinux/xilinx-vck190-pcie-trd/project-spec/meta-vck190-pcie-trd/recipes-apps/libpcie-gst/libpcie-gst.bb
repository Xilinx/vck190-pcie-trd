#
# This file is the libsample recipe.
#

SUMMARY = "Simple libpciegst application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://pcie_abstract.c \
	   file://Makefile \
	   file://pcie_abstract.h \
	  "

S = "${WORKDIR}"

inherit pkgconfig

DEPENDS = "gstreamer1.0 gstreamer1.0-plugins-base  v4l-utils libdrm glib-2.0"

do_compile() {
	     CFLAGS="${CFLAGS} `pkg-config --cflags glib-2.0`"
    	     LDFLAGS="${LDFLAGS} `pkg-config --libs glib-2.0`"
	     oe_runmake
}

do_install() {
	install -d ${D}${libdir}
    	install -d ${D}${includedir}
    	oe_libinstall -so libpciegst ${D}${libdir}
}
