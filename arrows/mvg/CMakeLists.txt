# Build / Install Multi-View Geometry arrow

set(CMAKE_FOLDER "Arrows/MVG")

set( plugin_mvg_headers )
set( plugin_mvg_sources )
set( plugin_mvg_classes
  algo/hierarchical_bundle_adjust
  algo/integrate_depth_maps
  algo/initialize_cameras_landmarks
  algo/initialize_cameras_landmarks_basic
  algo/triangulate_landmarks

  camera_options
  epipolar_geometry
  interpolate_camera
  necker_reverse
  metrics
  projected_track_set
  sfm_utils
  transform
  triangulate
  )

kwiver_expand_class_list( "${plugin_mvg_classes}" plugin_mvg_headers plugin_mvg_sources )

kwiver_install_headers(
  SUBDIR     arrows/mvg
  ${plugin_mvg_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_mvg_export.h
  NOPATH   SUBDIR     arrows/mvg
  )

# Address MSVC fatal error C1128:
# number of sections exceeded object file format limit
if(MSVC)
  set_source_files_properties(
    triangulate.cxx
    PROPERTIES COMPILE_FLAGS "/bigobj"
    )
endif()


kwiver_add_library( kwiver_algo_mvg
  ${plugin_mvg_headers}
  ${plugin_mvg_sources}
  )

target_link_libraries( kwiver_algo_mvg
  PUBLIC               vital_algo
  PRIVATE              kwiver_algo_core
                       kwiversys
                       vital_config
  )

algorithms_create_plugin( kwiver_algo_mvg
  algo/register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()

if (KWIVER_ENABLE_TOOLS)
  add_subdirectory(applets)
endif()
