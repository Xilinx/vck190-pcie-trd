# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.5)

project(smart-mipi-app)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -fpermissive -rdynamic -Wl,--no-undefined" )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(GStreamer REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME}
  gstapp-1.0 gstreamer-1.0 gstbase-1.0 gobject-2.0 glib-2.0 gstvideo-1.0 gstallocators-1.0)
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)

install(DIRECTORY
    config/yolov3
    config/facedetect
    config/refinedet
    config/ssd
    DESTINATION share/vvas/${CMAKE_PROJECT_NAME}/)

install(FILES
    config/image_processing.cfg
    DESTINATION share/vvas/${CMAKE_PROJECT_NAME}/)

install(DIRECTORY
    models/yolov3_voc_tf
    models/densebox_640_360
    models/refinedet_pruned_0_96
    models/ssd_adas_pruned_0_95
    DESTINATION share/vitis_ai_library/models/)

set(VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Xiyang@xilinx.com")
set(CPACK_PACKAGE_VENDOR "xilinx")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")
include(CPack)
