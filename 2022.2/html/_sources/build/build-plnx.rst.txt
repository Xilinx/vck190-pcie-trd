Build the PetaLinux Image
=========================

Prerequisites
-------------

* PetaLinux Tools 2022.2

* Linux host machine

Build Flow Tutorial
-------------------

**Clone the Reference Design Files:**

Skip this  step if the design has already been cloned and
extracted to a working directory

#. Clone the VCK190 PCIe TRD repository using the following command

   .. code-block:: bash

      git clone --branch 2022.2 --recursive https://github.com/Xilinx/vck190-pcie-trd.git

The directory structure is described in the Introduction Section.


**Generate PetaLinux Image**

Petalinux build flow has dependecy on artifacts from Vitis platform and overlay.
Complete the build steps listed in the below sections before generating a bootable petalinux image.

* Section 4.2: **Create the Vitis Platform**

  Ensure all steps in :ref:`Create a Vitis Extensible Platform` section are executed.

* Section 4.3: **Integrate Kernels in the Platform**

  Ensure all steps in the section :ref:`Implement platform design with filter2d PL and filter2d AIE accelerators` 
  are executed if filter2d is the targeted kernel and/or :ref:`Integrate XVDPU overlay` if xvdpu is the targeted kernel. 

The xsa and xclbin binaries produced from the previous step are consumed by
petalinux during the image creation step.

#. Source the PetaLinux tool settings.sh script.

#. Create a bootable SD card image:
   
   .. code-block:: bash

	make sdcard PFM=vck190_mipiRxSingle_hdmiTx OVERLAY=filter2d
   

The resulting build artifacts will be available in the *images/linux/* folder.
The following is a list of important output files:

* **binary_container_1.xclbin**: This is the kernel meta data file used by XRT

* **BOOT.BIN**: This is the boot image which includes:

  * Platform Loader and Manager (PLM)

  * PS Management (PSM) firmware

  * Platform Device Image (PDI)

  * ARM trusted firmware

  * u-boot

  * Device tree blob

  * Merged AI Engine application and configuration data object (CDO) file

* **boot.scr**: A u-boot boot script

* **Image**: Linux kernel image

* **ramdisk.cpio.gz.u-boot**: Initial filesystem in ramdisk

* **rootfs.tar.gz**: Compressed root file system tar ball

* **petalinux-sdimage.wic.xz**: Compressed SD card image file in wic format

The SD card image is now ready to be used to boot the device into Linux, see
Section :ref:`Run the Prebuilt Image` for details.

**Note:** 
To generate the SD card image for XVDPU kernel, create a copy of generated filter2d SD card image file, 
clean the petalinux project tmp space,
and repeat the steps by replacing *filter2d* with *xvdpu* in the make command.

Steps to Custom configure a PetaLinux project and build
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

User can configure BSP with custom XSA file, refer to the steps provided in

https://github.com/Xilinx/vck190-pcie-trd/blob/2022.2/petalinux/xilinx-vck190-pcie-trd/README.md

Reference
---------
Petalinux Tools Documentation

https://www.xilinx.com/support/documentation/sw_manuals/xilinx2022.2/ug1144-petalinux-tools-reference-guide.pdf

,,,,,

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License.

You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0


Unless required by applicable law or agreed to in writing, software distributed under the
License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing permissions
and limitations under the License.
