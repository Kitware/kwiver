#
# Top level for arrows
#
project( arrows )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( algorithms )

if (KWIVER_ENABLE_SPROKIT)
  # TODO add include_directories( sprokit )
  add_subdirectory( pipelines )
  add_subdirectory( processes )
endif()

set(arrows_DIR  ${CMAKE_CURRENT_BINARY_DIR}  CACHE PATH "Path to Arrows package")
