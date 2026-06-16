# Optional find and confgure VXL dependency

if(DEFINED fletch_ENABLED_VXL)
  set(_default ${fletch_ENABLED_VXL})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_VXL
  "Enable VXL dependent code and plugins (Arrows)"
  ${_default}
  )
unset(_default)

if( KWIVER_ENABLE_VXL )
  find_package( VXL REQUIRED )
  include(${VXL_CMAKE_DIR}/UseVXL.cmake)
  include_directories( SYSTEM ${VXL_CORE_INCLUDE_DIR} )
  include_directories( SYSTEM ${VXL_VCL_INCLUDE_DIR} )
  include_directories( SYSTEM ${VXL_RPL_INCLUDE_DIR} )
  link_directories( ${VXL_LIBRARY_DIR} )
endif( KWIVER_ENABLE_VXL )
