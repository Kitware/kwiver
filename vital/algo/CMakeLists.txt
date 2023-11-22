#
# CMake file for the vital tools
#

set(public_headers
   algorithm.h
   algorithm.txx
   video_input.h
  )
#
set(source_files
  algorithm.cxx
  video_input.cxx
  )

# algorithm library
kwiver_add_library( vital_algorithms
  ${public_headers}
  ${source_files}
)

target_link_libraries( vital_algorithms
  PUBLIC
              vital_vpm  # uses pluggable
              vital_types
 PRIVATE
              vital_logger
              vital
 )

###
## Install header files
kwiver_install_headers(
  ${public_headers}
  SUBDIR      vital/algo
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/vital_algo_export.h
  NOPATH
  SUBDIR      vital/algo
  )
