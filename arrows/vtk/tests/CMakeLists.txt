project(arrows_test_vtk)

set(CMAKE_FOLDER "Arrows/VTK/Tests")

include(kwiver-test-setup)

if(VTK_VERSION VERSION_LESS 9.0)
  set(test_libraries kwiver_algo_vtk vtkIOXML)
else()
  set(test_libraries kwiver_algo_vtk VTK::IOXML)
endif()

##############################
# VTK tests
##############################
kwiver_discover_gtests(vtk mesh_container LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(vtk mesh_utils     LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
