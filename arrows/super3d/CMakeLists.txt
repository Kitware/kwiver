
set(CMAKE_FOLDER "Arrows/Super3D")

set( plugin_super3d_headers )
set( plugin_super3d_sources
  warp_image.hxx
  warp_image_instances.cxx
  )
set( plugin_super3d_classes
  algo/compute_depth
  cost_volume
  tv_refine_search
  util
  warp_image
  world_angled_frustum
  world_space
)

kwiver_expand_class_list( "${plugin_super3d_classes}" plugin_super3d_headers plugin_super3d_sources )

kwiver_install_headers(
  SUBDIR     arrows/super3d
  ${plugin_super3d_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_super3d_export.h
  NOPATH   SUBDIR     arrows/super3d
  )

kwiver_add_library(kwiver_algo_super3d
  ${plugin_super3d_headers}
  ${plugin_super3d_sources}
  )
target_link_libraries(kwiver_algo_super3d
  PUBLIC              vital_algo
                      vil vpgl vnl vgl_algo
  PRIVATE             kwiver_algo_vxl
                      vil_algo vbl
  )

algorithms_create_plugin( kwiver_algo_super3d
  register_algorithms.cxx
  )


if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif(KWIVER_ENABLE_TESTS)
