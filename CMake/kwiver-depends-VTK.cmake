# Optional find and confgure VTK dependency

option( KWIVER_ENABLE_VTK
  "Enable VTK dependent code and plugins (Arrows)"
  ${fletch_ENABLED_VTK}
  )

if( KWIVER_ENABLE_VTK )
  find_package(VTK REQUIRED
    COMPONENTS
    vtkCommonCore
    vtkCommonDataModel
    )
  if(VTK_VERSION VERSION_LESS 8.2)
    message(FATAL_ERROR "${PROJECT_NAME} supports VTK >= v8.2 "
      "(Found ${VTK_VERSION})")
  elseif(NOT VTK_VERSION VERSION_LESS 8.90 )
    get_target_property(VTK_INCLUDE_DIR VTK::CommonCore INTERFACE_INCLUDE_DIRECTORIES)
    include_directories(SYSTEM "${VTK_INCLUDE_DIR}")
  else()
    include(${VTK_USE_FILE})
  endif()
endif( KWIVER_ENABLE_VTK )
