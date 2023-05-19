#
# PCIe GStreamer application recipe
#
SUMMARY = "PCIe GStreamer application"
SECTION = "PETALINUX/apps"
DESCRIPTION = "PCIe GStreamer application driven by a PCIe host application to run mipi and file display use-cases with and without filter"
LICENSE = "LGPLv2"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=b60ab04828851b8b8d8ad651889ac94d"

SRC_URI = " file://Makefile.am \
            file://configure.ac \
            file://autogen.sh \
            file://LICENSE.md \
            file://config \
            file://src/pcie_main.c \
            file://src/pcie_src.c \
            file://src/pcie_sink.c \
            file://src/dpu_pipe.cpp \
            file://src/pcie_abstract.c \
            file://include/pcie_main.h \
            file://include/pcie_src.h \
            file://include/pcie_abstract.h \
            file://include/pcie_sink.h \
            file://include/dpu_pipe.h \
          "

#FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

S = "${WORKDIR}"

CFLAGS:prepend = "-I${S}/include/ -O0"
CPPFLAGS:prepend = "-I${S}/include/ -O0"

DEPENDS = "gstreamer1.0 gstreamer1.0-plugins-base v4l-utils libdrm"

# NOTE: if this software is not capable of being built in a separate build directory
# from the source, you should replace autotools with autotools-brokensep in the
# inherit line
inherit pkgconfig autotools

# Specify any options you want to pass to the configure script using EXTRA_OECONF:
EXTRA_OECONF = ""

FILES:${PN} += "/usr/bin/pcie_gst_app /usr/share/vvas/pcie-gst-app"

do_install() {
	install -d ${D}/${bindir}
	oe_runmake install DESTDIR=${D}
	install -d ${D}/usr/share/vvas/pcie-gst-app
	install -m 0755 ${B}/pcie_gst_app ${D}/${bindir}
        cp -R --no-dereference --preserve=mode,links -v ${S}/config/*  ${D}/usr/share/vvas/pcie-gst-app/
#        cp -R --no-dereference --preserve=mode,links -v ${S}/config/image_processing.cfg  ${D}/run/media/mmcblk0p1/
}
