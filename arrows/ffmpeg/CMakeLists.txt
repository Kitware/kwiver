# Build / Install plugin containing ffmpeg algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/FFmpeg")

if(NOT KWIVER_ENABLE_KLV)
  message(FATAL_ERROR
          "The FFmpeg arrow requires the KLV arrow (KWIVER_ENABLE_KLV)")
endif()

include_directories(${FFMPEG_INCLUDE_DIR})

if (NOT FFMPEG_FOUND_SEVERAL)
  message( FATAL_ERROR "FFmpeg headers all in the same directories, "
"this is not supported by this Arrow."
"\nThis likely indicates that you are building against an old FFmpeg ! "
)
endif()

set(ffmpeg_headers_public
  ffmpeg_audio_stream_settings.h
  ffmpeg_cuda.h
  ffmpeg_init.h
  ffmpeg_util.h
  ffmpeg_video_input.h
  ffmpeg_video_output.h
  ffmpeg_video_raw_image.cxx
  ffmpeg_video_raw_metadata.cxx
  ffmpeg_video_settings.h
  ffmpeg_video_uninterpreted_data.h
  )

kwiver_install_headers(
  SUBDIR     arrows/ffmpeg
  ${ffmpeg_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_ffmpeg_export.h
  NOPATH   SUBDIR     arrows/ffmpeg
  )

set(ffmpeg_sources
  ffmpeg_audio_stream_settings.cxx
  ffmpeg_cuda.cxx
  ffmpeg_init.cxx
  ffmpeg_util.cxx
  ffmpeg_video_input.cxx
  ffmpeg_video_output.cxx
  ffmpeg_video_raw_image.cxx
  ffmpeg_video_raw_metadata.cxx
  ffmpeg_video_settings.cxx
  ffmpeg_video_uninterpreted_data.cxx
  )

if(KWIVER_ENABLE_CUDA)
  set(KWIVER_ENABLE_FFMPEG_CUDA ON CACHE BOOL "Enable use of CUVID/NVENC codecs")
  if(KWIVER_ENABLE_FFMPEG_CUDA)
    add_compile_definitions(KWIVER_ENABLE_FFMPEG_CUDA)
    set(FFMPEG_CUDA_LIBRARIES cuda)
  endif()
endif()

kwiver_add_library( kwiver_algo_ffmpeg
  ${ffmpeg_headers_public}
  ${ffmpeg_sources}
  )

if(fletch_ENABLED_ZLib)
  if(KWIVER_BUILD_SHARED)
    find_package(ZLIB MODULE REQUIRED)
  else()
    # Zlib produces both static and dynamic libraries
    # This allows static libraries to be picked instead of shared libraries
    # Thereby not creating a zlib dependency in he library
    find_library(ZLIB_LIBRARIES NAMES libz.a libz libzlib
                 PATHS ${fletch_ROOT}
                 PATH_SUFFIXES lib
                 )
  endif()
endif()

target_link_libraries( kwiver_algo_ffmpeg
  PUBLIC               vital_algo
  PRIVATE              kwiver_algo_klv kwiversys
                       ${FFMPEG_CUDA_LIBRARIES}
                       ${FFMPEG_LIBRARIES}
                       ${ZLIB_LIBRARIES}
  )


algorithms_create_plugin( kwiver_algo_ffmpeg
  register_algorithms.cxx
  )


if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
