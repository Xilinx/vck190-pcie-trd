bin_PROGRAMS = pcie_gst_app

AM_DEFAULT_SOURCE_EXT = .c .cpp

pcie_gst_app_INC_DIR = ./include
pcie_gst_app_SRC_DIR  = ./src

PCI_CFLAGS_PKG_CONFIG = $$(pkg-config --cflags  gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-app-1.0 libdrm libv4l2 libmediactl)
PCI_LIBS_PKG_CONFIG = $$(pkg-config --libs gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-app-1.0 gstreamer-video-1.0 libdrm libmediactl libv4l2)


pcie_gst_app_SOURCES = $(pcie_gst_app_SRC_DIR)/pcie_main.c \
                       $(pcie_gst_app_SRC_DIR)/pcie_src.c \
                       $(pcie_gst_app_SRC_DIR)/pcie_abstract.c \
                       $(pcie_gst_app_SRC_DIR)/pcie_sink.c \
                       $(pcie_gst_app_SRC_DIR)/dpu_pipe.cpp

nodist_pcie_gst_app_SOURCES = $(pcie_gst_app_INC_DIR)/pcie_main.h \
                              $(pcie_gst_app_INC_DIR)/pcie_src.h \
                              $(pcie_gst_app_INC_DIR)/pcie_abstract.h \
                              $(pcie_gst_app_INC_DIR)/pcie_sink.h \
                              $(pcie_gst_app_INC_DIR)/dpu_pipe.h

pcie_gst_app_CFLAGS = $(PCI_CFLAGS_PKG_CONFIG) -I$(pcie_gst_app_INC_DIR) -g
pcie_gst_app_CPPFLAGS = $(PCI_CFLAGS_PKG_CONFIG) -I$(pcie_gst_app_INC_DIR) -g

pcie_gst_app_LDFLAGS = $(PCI_LIBS_PKG_CONFIG) -lm  -lgstvideo-1.0 -lgstallocators-1.0
