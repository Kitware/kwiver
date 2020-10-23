# Optional find and confgure VTK dependency

option( KWIVER_ENABLE_VTK
  "Enable VTK dependent code and plugins (Arrows)"
  ${fletch_ENABLED_VTK}
  )

if( KWIVER_ENABLE_VTK )
    find_package(VTK 8.2
        COMPONENTS
        vtkCommonCore
        vtkCommonDataModel
        )
    if(VTK_VERSION VERSION_LESS 8.2)
        message(WARNING  "${PROJECT_NAME} supports VTK >= v8.2 "
          "(Found ${VTK_VERSION}). Disabling VTK support")
        set( KWIVER_ENABLE_VTK OFF CACHE BOOL "" FORCE)
      else()
        include(${VTK_USE_FILE})
      endif()
endif( KWIVER_ENABLE_VTK )
