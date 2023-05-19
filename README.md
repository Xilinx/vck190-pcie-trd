# VCK190 PCIe TRD

This repo uses git submodules. To clone this repo, run

``` bash
git clone --recursive https://github.com/Xilinx/vck190-pcie-trd.git
```

## Tools Version

The supported Xilinx tools release is 2022.2.

## Overview

The Versal PCIe TRD consists of a platform, accelerators and Jupyter notebooks to demonstrate various aspects of the design. The platform is a Vivado design with a pre-instantiated set of I/O interfaces and a corresponding PetaLinux BSP and image that includes the required kernel drivers and user-space libraries to exercise those interfaces. Accelerators are mapped to FPGA logic resources and/or AI Engine cores and stitched into the platform using the Vitis toolchain. The reference designs are implemented for VCK190 evaluation board.

## Features

VCK190 PCIe TRD 2022.2 demonstrates following functionalities :

* Video being captured from MIPI image sensor, processed with an accelerator and displayed on a HDMI monitor connected to VCK190 evaluation board.
* Video being captured from MIPI image sensor, processed (optionally) with an accelerator, sent to a x86 host machine (root complex) via PCIe and displayed on a monitor connected to the host machine.
* Video frames being transferred from a file on the host machine to the VCK190 evaluation board (endpoint) through the PCIe QDMA bridge interface, processed (optionally) with an accelerator, sent back to host and displayed on a monitor connected to the host machine. . 
* Video frames being transferred from a file on the host machine to the VCK190 evaluation board (endpoint) through the PCIe QDMA bridge interface, processed (optionally) with an accelerator and displayed on HDMI monitor connected to VCK190 board.


## Platform

The Vitis platform captures video from either a file source, or an image sensor using the [IMX274 MIPI FMC Module](https://www.leopardimaging.com/uploads/LI-IMX274MIPI-FMC_datasheet.pdf) or a PCIe based video source and displays it on a HDMI monitor or sends back to host via PCIe where it can be displayed on a monitor. Accelerator functions can be added into this platform using the Vitis acceleration flow. 

## Overlays

1. **Filter2d:** This overlay adds a 2D convolution filter with programmable coefficients into a Vitis platform. The current 2D filter implementation supports an OpenCV based version running on the APU and an HLS-based PL accelerated version using the Vitis Vision libraries. A series of jupyter notebooks are provided demonstrating different data flows through the 2D filter.

2. **XVDPU:** This overlay adds the XVDPU ML inference engine and multi-scaler IP into a Vitis platform. A command line application allows capturing video from the image sensor(s), running the ML model(s) and then displaying the output on the display. The supported ML models are: yolov3, refinedet, densebox, ssd.

## Documentation

Additional documentation can be found on [github pages](https://xilinx.github.io/vck190-pcie-trd/2022.2/html/index.html) including:

* Tutorials for running the prebuilt SD card images and applications
* Tool flow tutorials for various design components including Vitis platforms, Vitis overlays and the PetaLinux BSP
* Architecture guide on hardware designs and software stack

## Third-Party Content

The design includes files licensed by Xilinx, AMD, and third parties under the terms of the GNU General Public License, GNU Lesser General Public License, BSD License, MIT License, and other licenses. All Xilinx, AMD, and third-party licenses and sources associated with this reference design can be downloaded [here](https://www.xilinx.com/bin/public/openDownload?filename=vck190_pcie_trd_third_party_licenses_and_source_2022_2.tar.gz). Package includes one folder named `sources` containing the complete set of source files and one named `licenses` containing licenses extracted from the source files. These licenses and sources are extracted from petalinux bsp recipes. You are solely responsible for checking any files you use for notices and licenses and for complying with any terms applicable to your use of the design and any third party files supplied with the design.

To generate licenses and sources for petalinux BSP use following command.

`petalinux-build -a`

To obtain Xilinx image.ub licenses files: Licenses for image.ub files are included in the `/usr/share/licenses` directory when the image file is built. DNF package manager can be used to list all packages in image as well as download all the sources for all the packages.

## License

Licensed under the Apache License, version 2.0 (the "License"); you may not use this file except in compliance with the License.

You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
