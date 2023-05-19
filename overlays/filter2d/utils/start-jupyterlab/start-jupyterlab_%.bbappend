FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-recipes-utils-Add-jupyter-server-config-file.patch \
	    file://jupyter-setup.service \
		"

SYSTEMD_PACKAGES="${PN}"
SYSTEMD_SERVICE:${PN}="jupyter-setup.service"
SYSTEMD_AUTO_ENABLE:${PN}="enable"

do_install:append() {
     install -d ${D}${datadir}/notebooks
     install -d ${D}${sysconfdir}/jupyter/
     install -m 0644 ${WORKDIR}/jupyter_server_config.py ${D}${sysconfdir}/jupyter
     install -d ${D}${systemd_system_unitdir}
     install -m 0644 ${WORKDIR}/jupyter-setup.service ${D}${systemd_system_unitdir}

}

FILES:${PN} += "${datadir}/notebooks \
		${systemd_user_unitdir}"
