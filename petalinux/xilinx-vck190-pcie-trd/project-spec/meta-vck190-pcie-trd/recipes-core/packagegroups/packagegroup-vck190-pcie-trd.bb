DESCRIPTION = "vck190 pcie TRD related Packages"

COMPATIBLE_MACHINE:versal = ".*"

inherit packagegroup

VCK190-PCIE_TRD_PACKAGES = " \
	packagegroup-core-tools-debug \
	packagegroup-petalinux-display-debug \
	packagegroup-petalinux-gstreamer \
	packagegroup-petalinux-jupyter \
	packagegroup-petalinux-opencv \
	packagegroup-petalinux-python-modules \
	packagegroup-petalinux-self-hosted \
	packagegroup-petalinux-vitisai \
	packagegroup-petalinux-v4lutils \
	cmake \
	gstreamer1.0-python \
	kernel-module-hdmi \
	ldd \
	libxperfmon-python \
	ntp \
	python3-dev \
	python3-periphery \
	resize-part \
	tcf-agent \
	trd-dashboard \
	trd-files \
	tree \
	ttf-bitstream-vera \
	tzdata \
	xrt \
	gst-plugins-xlnx \
	xmediactl \
	pcie-gst-app \
	libpcie-gst \
	libxapm \
	vvas-accel-libs \
	vvas-gst \            
	vvas-core \            
	vvas-utils \            
	"

RDEPENDS:${PN} = "${VCK190-PCIE_TRD_PACKAGES}"
