#
# This file is the xmedia-ctl recipe.
#

SUMMARY = "Simple xmedia-ctl application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://xmedia-ctl \
	   file://xmediactl.sh \
	   file://xmediactl_1080.sh \
	   file://xmediactl-rgb.sh \
	   file://xmediactl-rgb-1080.sh \
	"

S = "${WORKDIR}"

do_install() {
	     install -d ${D}/usr/bin
	     install -m 0755 ${S}/xmedia-ctl ${D}/usr/bin
	     install -m 0755 ${S}/xmediactl.sh ${D}/usr/bin
	     install -m 0755 ${S}/xmediactl_1080.sh ${D}/usr/bin
	     install -m 0755 ${S}/xmediactl-rgb.sh ${D}/usr/bin
	     install -m 0755 ${S}/xmediactl-rgb-1080.sh ${D}/usr/bin
}
FILES:${PN} += ""
INSANE_SKIP:${PN} = "file-rdeps"
