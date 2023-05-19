# Source files
add_library (gstxlnxvideosrc SHARED
  gstxlnxvideosrc.c
)

# Dependencies
find_library (V4L2SUBDEV_LIBRARIES v4l2subdev)
find_library (MEDIACTL_LIBRARIES mediactl)
find_package (GLIB REQUIRED)
find_package (Threads REQUIRED)
find_package (LibDRM REQUIRED)
set(GStreamer_FIND_VERSION "1.8.3")
find_package (GStreamer REQUIRED Video)

target_include_directories (gstxlnxvideosrc
  PUBLIC ${GSTREAMER_VIDEO_INCLUDE_DIRS}
)

target_link_libraries (gstxlnxvideosrc
  video
  ${MEDIACTL_LIBRARIES}
  ${V4L2SUBDEV_LIBRARIES}
  ${LIBDRM_LIBRARIES}
  ${GLIB_GIO_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

install (TARGETS gstxlnxvideosrc DESTINATION ${GSTSDX_PLUGIN_INSTALL_DIR})
