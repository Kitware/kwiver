# Build / Install Key-Length-Value Metadata Arrow

set( CMAKE_FOLDER "Arrows/KLV" )

set( sources
  bytestream_compressor.cxx
  )

set( public_headers
  bytestream_compressor.h
  )

set( private_headers )

kwiver_install_headers(
  ${public_headers}
  SUBDIR arrows/zlib
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_zlib_export.h
  NOPATH
  SUBDIR arrows/zlib
  )


if(NOT KWIVER_BUILD_SHARED)
  if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  endif()
endif()
find_package(ZLIB MODULE REQUIRED)

kwiver_add_library( kwiver_algo_zlib
  ${sources}
  ${public_headers}
  ${private_headers}
  )

target_link_libraries( kwiver_algo_zlib
  PRIVATE
    vital
    ZLIB::ZLIB
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory( tests )
endif()
