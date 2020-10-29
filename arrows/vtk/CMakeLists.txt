# Build / Install VTK arrow

set( plugin_vtk_headers
  depth_utils.h
  )

kwiver_install_headers(
  SUBDIR     arrows/vtk
  ${plugin_vtk_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_vtk_export.h
  NOPATH   SUBDIR     arrows/vtk
  )

set( plugin_vtk_sources
  depth_utils.cxx
  )

if (NOT VTK_VERSION VERSION_LESS 8.90)
  set (VTK_ARROW_LIBS
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::RenderingCore
    )
else()
  set (VTK_ARROW_LIBS
    vtkCommonCore
    vtkCommonDataModel
    vtkRenderingCore
    )
endif()

kwiver_add_library( kwiver_algo_vtk
  ${plugin_vtk_headers}
  ${plugin_vtk_sources}
  )

target_link_libraries( kwiver_algo_vtk
  PUBLIC               vital
                       ${VTK_ARROW_LIBS}
  )

#algorithms_create_plugin( kwiver_algo_vtk
#  register_algorithms.cxx
#  )

if (KWIVER_ENABLE_TESTS)
#  add_subdirectory(tests)
endif()

if (KWIVER_ENABLE_TOOLS)
#  add_subdirectory(applets)
endif()
