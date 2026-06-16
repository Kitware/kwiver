# Optional find and confgure VTK dependency

if(DEFINED fletch_ENABLED_VTK)
  set(_default ${fletch_ENABLED_VTK})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_VTK
  "Enable VTK dependent code and plugins (Arrows)"
  ${_default}
  )
unset(_default)

if( KWIVER_ENABLE_VTK )
  find_package(VTK)
  if(VTK_VERSION VERSION_LESS 9.0)
    find_package(VTK REQUIRED
        COMPONENTS
        vtkCommonCore
        vtkCommonDataModel
        vtkIOXML
        vtkIOPLY
        vtkIOGeometry
        vtkRenderingCore
        vtkRenderingOpenGL2
        )
    if(VTK_VERSION VERSION_LESS 8.2)
        message(FATAL_ERROR "${PROJECT_NAME} supports VTK >= v8.2 "
            "(Found ${VTK_VERSION})")
    endif()

    include(${VTK_USE_FILE})
  else()
    find_package(VTK REQUIRED
        COMPONENTS
        CommonCore
        CommonDataModel
        IOXML
        IOPLY
        IOGeometry
        RenderingCore
        RenderingOpenGL2
        )
  endif()

endif( KWIVER_ENABLE_VTK )
