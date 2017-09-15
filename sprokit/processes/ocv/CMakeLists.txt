#
# KWIVER processes that use OpenCV
#
project( kwiver_ocv_processes )

set( sources
  register_processes.cxx

  blend_images_process.cxx
  crop_image_process.cxx
  draw_detected_object_boxes_process.cxx
  equalize_histogram_process.cxx
  image_viewer_process.cxx
  show_mask_on_image_process.cxx
  stabilization_inspection_image_process.cxx
)

set( private_headers
  blend_images_process.h
  crop_image_process.h
  draw_detected_object_boxes_process.h
  equalize_histogram_process.h
  image_viewer_process.h
  show_mask_on_image_process.h
  stabilization_inspection_image_process.h
)

kwiver_private_header_group( ${private_headers} )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

find_package( OpenCV REQUIRED )
if( OpenCV_VERSION VERSION_LESS "2.4" )
  message( FATAL_ERROR "OpenCV version must be at least 2.4" )
endif()

kwiver_add_plugin( kwiver_processes_ocv
  SUBDIR           sprokit
  SOURCES        ${sources}
                 ${private_headers}
  PRIVATE          sprokit_pipeline
                   kwiver_algo_core kwiver_algo_ocv
                   vital vital_vpm vital_logger vital_config
                   kwiversys
                 ${OpenCV_LIBS}
 )
