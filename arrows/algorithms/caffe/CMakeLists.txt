# Build / Install plugin containing OpenCV algorithm implementations + support
# structures

set( caffe_headers_public
  faster_rcnn_detector.h
  register_algorithms.h
  )
kwiver_install_headers(
  SUBDIR     arrows/algorithms/caffe
  ${caffe_headers_public}
  )
kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/algorithms_caffe_export.h
  NOPATH   SUBDIR     arrows/algorithms/caffe
  )

set( caffe_sources
  faster_rcnn_detector.cxx
  register_algorithms.cxx
  )
kwiver_add_library( algorithms_caffe
  ${caffe_headers_public}
  ${caffe_sources}
  )
target_link_libraries( algorithms_caffe
  PUBLIC               algorithms_ocv
                       algorithms_core
                       kwiversys
                       ${OpenCV_LIBS} caffe
  )

algorithms_create_plugin( algorithms_caffe
  algorithm_plugin_interface.cxx
  )

if (KWIVER_ENABLE_TESTS)
  #add_subdirectory(tests)
endif()
