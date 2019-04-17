# Build / Install plugin containing mie4nitf algorithm implementations + support
# structures

find_package(LibXml2 REQUIRED)
list(APPEND MIE4NITF_INCLUDE_DIR ${LIBXML2_INCLUDE_DIR})
include_directories(${MIE4NITF_INCLUDE_DIR})

set(mie4nitf_headers_public
  mie4nitf_init.h
  mie4nitf_video_input.h
  )

kwiver_install_headers(
  SUBDIR     arrows/mie4nitf
  ${mie4nitf_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_mie4nitf_export.h
  NOPATH   SUBDIR     arrows/mie4nitf
  )

set(mie4nitf_sources
  mie4nitf_init.cxx
  mie4nitf_video_input.cxx
  )

kwiver_add_library( kwiver_algo_mie4nitf
  ${mie4nitf_headers_public}
  ${mie4nitf_sources}
  )

target_link_libraries( kwiver_algo_mie4nitf
  PUBLIC               kwiver_algo_core kwiver_algo_gdal
  PRIVATE              kwiversys ${GDAL_LIBRARY} ${LIBXML2_LIBRARIES}
  )

algorithms_create_plugin( kwiver_algo_mie4nitf
  register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
