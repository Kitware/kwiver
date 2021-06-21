# TODO Make everything configurable with variables so developers dont have to reconfigure everytime
set(plugin_arrowtemplate_headers
    arrowtemplate.h
)

kwiver_install_headers(
    SUBDIR arrows/arrowtemplate
    ${plugin_arrowtemplate_headers}
)

kwiver_install_headers(
    ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_arrowtemplate_export.h
    NOPATH SUBDIR arrows/arrowtemplate
)

set(
    plugin_arrowtemplate_sources
    arrowtemplate.cpp
)

kwiver_add_library(
    kwiver_algo_arrowtemplate
    ${plugin_arrowtemplate_headers}
    ${plugin_arrowtemplate_sources}
)

target_link_libraries(
    kwiver_algo_arrowtemplate
    PUBLIC vital
)

# There are also changes to the CMakeLists.txt 1 level up that need to be made