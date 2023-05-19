# gst-plugins-xlnx
GStreamer Xilinx plugins

## Cmake build flow

### Prerequisites
Source the Petalinux environment setup script to set up the SYSROOT for
cross-compilation. The environment script is packaged as part of the sdk.sh
installer.

### Example
```bash
source environment-setup-aarch64-xilinx-linux
mkdir build
cd build
CC=/path/to/gcc CXX=/path/to/g++ cmake ..
make
```
