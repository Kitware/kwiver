project(arrows_test_vtk)

set(CMAKE_FOLDER "Arrows/VTK/Tests")

include(kwiver-test-setup)

if(VTK_VERSION VERSION_LESS 9.0)
  set(test_libraries kwiver_algo_vtk vtkIOXML vtkFiltersSources)
else()
  set(test_libraries kwiver_algo_vtk VTK::IOXML VTK::FiltersSources)
endif()

##############################
# VTK tests
##############################
kwiver_discover_gtests(vtk decimate_mesh_filter    LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(vtk mesh_container          LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(vtk mesh_utils              LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(vtk smooth_mesh_filter      LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(vtk triangulate_mesh_filter LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
