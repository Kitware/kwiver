###
# algorithms/proj

set(proj_headers_public
  )

kwiver_install_headers(
  SUBDIR     arrows/proj
  ${proj_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_arrows_proj_export.h
  NOPATH   SUBDIR     arrows/proj
  )

set(proj_sources
  geo_conv.cxx
  )

kwiver_add_library( kwiver_arrows_proj
  ${proj_headers_public}
  ${proj_sources}
  )

target_link_libraries( kwiver_arrows_proj
  PUBLIC               kwiver_arrows_core
                       ${PROJ_LIBRARY}
  )

algorithms_create_plugin( kwiver_arrows_proj
  register_algorithms.cxx
  )
