SUMMARY = "Smart mipicam application"
DESCRIPTION = "Application for runing single/quad mipi sensor with Vitis AI/VVAS"
SECTION = "PETALINUX/apps"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=055f183b6a3144393f20b5191577a4e8"

SRC_URI = " \
    file://cmake \
    file://config \
    file://models \
    file://src \
    file://CMakeLists.txt \
    file://LICENSE \
    "

S = "${WORKDIR}"

DEPENDS = " glog vvas-accel-libs opencv"

RDEPENDS:${PN} += " \
	gst-perf \
	gstreamer1.0-plugins-bad-kms \
	gstreamer1.0-plugins-bad-mediasrcbin \
	gstreamer1.0-plugins-bad-videoparsersbad \
	gstreamer1.0-plugins-good-multifile \
	gstreamer1.0-plugins-good-video4linux2 \
	vvas-accel-libs \
	libdrm-tests \
	v4l-utils \
	alsa-utils \
	"

inherit cmake

EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=Release"

do_install:append() {
	install -d ${D}${sysconfdir}/init.d
}

FILES:${PN} += " \
    ${datadir} \
    "
