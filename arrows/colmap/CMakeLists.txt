# TODO Make everything configurable with variables so developers dont have to reconfigure everytime
set(plugin_colmap_headers
    colmap.h
)

kwiver_install_headers(
    SUBDIR arrows/colmap
    ${plugin_colmap_headers}
)

kwiver_install_headers(
    ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_colmap_export.h
    NOPATH SUBDIR arrows/colmap
)

set(
    plugin_colmap_sources
    colmap.cxx
)

kwiver_add_library(
    kwiver_algo_colmap
    ${plugin_colmap_headers}
    ${plugin_colmap_sources}
)

target_include_directories(
    kwiver_algo_colmap PUBLIC
    ${COLMAP_INCLUDE_DIRS}
)

target_link_directories(
    kwiver_algo_colmap PUBLIC
    ${COLMAP_LINK_DIRS}
)

target_link_libraries(
    kwiver_algo_colmap
    PUBLIC vital
    ${COLMAP_LIBRARIES}
)
