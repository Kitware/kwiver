# Build / Install plugin containing OpenCV algorithm implementations + support
# structures
set( caffe_headers_public
  faster_rcnn_detector.h
  )

kwiver_install_headers(
  SUBDIR     arrows/caffe
  ${caffe_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_caffe_export.h
  NOPATH   SUBDIR     arrows/caffe
  )

set( caffe_sources
  faster_rcnn_detector.cxx
  )

kwiver_add_library( kwiver_algo_caffe
  ${caffe_headers_public}
  ${caffe_sources}
  )

target_link_libraries( kwiver_algo_caffe
  PUBLIC               kwiver_algo_core
                       caffe
  PRIVATE              kwiver_algo_ocv
                       kwiversys
                       ${OpenCV_LIBS}
  )

algorithms_create_plugin( kwiver_algo_caffe
  register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  #add_subdirectory(tests)
endif()
