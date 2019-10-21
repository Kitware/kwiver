# Build / Install plugin containing ffmpeg algorithm implementations + support
# structures

include_directories(${FFMPEG_INCLUDE_DIR})

if (NOT FFMPEG_FOUND_SEVERAL)
  message( FATAL_ERROR "FFMPEG headers all in the same directories, "
"this is not supported by this Arrow."
"\nThis likely indicates that you are building against an old FFMPEG ! "
)
endif()

# ffmpeg library implementation
set(ffmpeg_impl_headers_public
    ffmpeg_init.h
    ffmpeg_video_input_impl.h
   )

set(ffmpeg_impl_sources
    ffmpeg_init.cxx
    ffmpeg_video_input_impl.cxx
   )

kwiver_install_headers(
  SUBDIR     arrows/ffmpeg
  ${ffmpeg_impl_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_ffmpeg_impl_export.h
  NOPATH   SUBDIR     arrows/ffmpeg
  )

kwiver_add_library( kwiver_algo_ffmpeg_impl
  ${ffmpeg_impl_headers_public}
  ${ffmpeg_impl_sources}
  )

target_link_libraries( kwiver_algo_ffmpeg_impl
   PUBLIC              vital
   PRIVATE             vital_klv kwiversys
                       vital_logger
                       ${FFMPEG_LIBRARIES}
  )

# ffmpeg arrow library
set(ffmpeg_headers_public
  ffmpeg_video_input.h
  )

set(ffmpeg_sources
  ffmpeg_video_input.cxx
  )

kwiver_install_headers(
  SUBDIR     arrows/ffmpeg
  ${ffmpeg_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_ffmpeg_export.h
  NOPATH   SUBDIR     arrows/ffmpeg
  )


kwiver_add_library( kwiver_algo_ffmpeg
  ${ffmpeg_headers_public}
  ${ffmpeg_sources}
  )

if(fletch_ENABLED_ZLib)
  if(KWIVER_BUILD_SHARED)
    find_package(ZLIB MODULE REQUIRED)
  else()
    find_library(ZLIB_LIBRARIES NAMES libz.a libz libzlib
                 PATHS ${fletch_ROOT}
                 PATH_SUFFIXES lib
                 )
  endif()
endif()

set_target_properties(kwiver_algo_ffmpeg PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic")

target_link_libraries( kwiver_algo_ffmpeg
  PUBLIC               kwiver_algo_core
                       kwiver_algo_ffmpeg_impl
  PRIVATE              vital_klv kwiversys
                       ${FFMPEG_LIBRARIES}
                       ${ZLIB_LIBRARIES}
  )


algorithms_create_plugin( kwiver_algo_ffmpeg
  register_algorithms.cxx
  )

set_target_properties(kwiver_algo_ffmpeg_plugin PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic")

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
