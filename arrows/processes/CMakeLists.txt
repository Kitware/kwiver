#
# Arrows processes
#

add_subdirectory( bindings )
add_subdirectory( core )
add_subdirectory( adapters )
add_subdirectory( examples )

if (KWIVER_ENABLE_VXL)
  add_subdirectory( vxl )
endif()

if (KWIVER_ENABLE_OPENCV)
  add_subdirectory( ocv )
endif()

if (KWIVER_ENABLE_PYTHON)
  add_subdirectory( python )
endif()

if (KWIVER_ENABLE_CAFFE)
#  add_subdirectory(caffe)
endif()
