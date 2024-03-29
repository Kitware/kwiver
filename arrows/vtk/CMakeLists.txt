# Build / Install VTK arrow

set(CMAKE_FOLDER "Arrows/VTK")

set( plugin_vtk_headers
  depth_utils.h
  vtkKwiverCamera.h
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
  vtkKwiverCamera.cxx
  )

if (VTK_VERSION VERSION_GREATER_EQUAL 9.1)
  list(APPEND plugin_vtk_headers mesh_coloration.h)
  list(APPEND plugin_vtk_sources mesh_coloration.cxx)
  add_definitions( -DVTK_ENABLE_COLOR_MESH )
endif()

kwiver_add_library( kwiver_algo_vtk
  ${plugin_vtk_headers}
  ${plugin_vtk_sources}
  )

if(VTK_VERSION VERSION_LESS 9.0)
  set(VTK_public_targets
      vtkRenderingCore
      vtkRenderingOpenGL2)
  set(VTK_private_targets
      vtkCommonCore
      vtkCommonDataModel
      vtkIOImage
      vtkIOXML)
else()
  set(VTK_public_targets
      VTK::RenderingCore
      VTK::RenderingOpenGL2)
  set(VTK_private_targets
      VTK::CommonCore
      VTK::CommonDataModel
      VTK::IOImage
      VTK::IOXML)
  vtk_module_autoinit(
    TARGETS kwiver_algo_vtk
    MODULES ${VTK_public_targets} ${VTK_private_targets})
endif()

target_link_libraries( kwiver_algo_vtk
  PUBLIC               vital
                       ${VTK_public_targets}
  PRIVATE              vital_algo
                       ${VTK_private_targets}
  )

#algorithms_create_plugin( kwiver_algo_vtk
#  register_algorithms.cxx
#  )

if (KWIVER_ENABLE_TESTS)
#  add_subdirectory(tests)
endif()

if (KWIVER_ENABLE_TOOLS)
  add_subdirectory(applets)
endif()
