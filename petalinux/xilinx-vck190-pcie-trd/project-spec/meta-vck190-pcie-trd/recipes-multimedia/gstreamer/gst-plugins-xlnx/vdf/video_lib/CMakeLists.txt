cmake_minimum_required (VERSION 2.8.12)

project (video_lib C)

# Cmake options
option (DEBUG_MODE "Enable debug output" OFF)
option (ENABLE_VCAP_UVC "Enable video source USB Video Class" ON)
option (ENABLE_VCAP_VIVID "Enable video source Vivid virtual video device" ON)
option (ENABLE_VCAP_FILE "Enable video source File" ON)

# Source files
add_library (video STATIC
	src/drm_helper.c
	src/gpio_utils.c
	src/mediactl_helper.c
	src/qos.c
	src/v4l2_helper.c
	src/vcap_csi.c
	src/vcap_gmsl.c
	src/vcap_file.c
	src/vcap_hdmi.c
	src/vcap_tpg.c
	src/vcap_uvc.c
	src/vcap_vivid.c
	src/video.c
	src/video_src.c
)

# Dependencies
find_path (V4L2SUBDEV_INCLUDE_DIRS mediactl/v4l2subdev.h)
find_path (MEDIACTL_INCLUDE_DIRS mediactl/mediactl.h)
find_package (GLIB REQUIRED)
find_package (LibDRM REQUIRED)

target_compile_definitions (video
	PRIVATE ${VDF_CDEFS_PFM}
	PRIVATE $<$<CONFIG:Release>:${VDF_CDEFS_REL}>
	PRIVATE $<$<CONFIG:RelWithDebInfo>:${VDF_CDEFS_REL}>
	PRIVATE $<$<BOOL:${DEBUG_MODE}>:DEBUG_MODE>
	PRIVATE $<$<BOOL:${ENABLE_VCAP_UVC}>:ENABLE_VCAP_UVC>
	PRIVATE $<$<BOOL:${ENABLE_VCAP_VIVID}>:ENABLE_VCAP_VIVID>
	PRIVATE $<$<BOOL:${ENABLE_VCAP_FILE}>:ENABLE_VCAP_FILE>
	PRIVATE VCK190_PCIE_TRD
)

target_compile_options (video
	PRIVATE ${VDF_CFLAGS} -fPIC -fvisibility=hidden
	PRIVATE $<$<CONFIG:Debug>:${VDF_CFLAGS_DGB}>
	PRIVATE $<$<CONFIG:Release>:${VDF_CFLAGS_REL}>
	PRIVATE $<$<CONFIG:RelWithDebInfo>:${VDF_CFLAGS_RELDBG}>
)

target_include_directories (video
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include
	PRIVATE ${LIBDRM_INCLUDE_DIRS}
	PRIVATE ${MEDIACTL_INCLUDE_DIRS}
	PRIVATE ${V4L2SUBDEV_INCLUDE_DIRS}
	PUBLIC ${GLIB_INCLUDE_DIRS}
)

target_link_libraries (video)

install (TARGETS video EXPORT video_lib DESTINATION lib)
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
export (EXPORT video_lib FILE video-config.cmake)
