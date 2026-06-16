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

set( plugin_ffmpeg_headers )
set( plugin_ffmpeg_sources
  ffmpeg_convert_image.cxx
  )
set( plugin_ffmpeg_classes
  algo/ffmpeg_image_io
  algo/ffmpeg_video_input
  algo/ffmpeg_video_input_clip
  algo/ffmpeg_video_output

  ffmpeg_audio_stream_settings
  ffmpeg_cuda
  ffmpeg_init
  ffmpeg_util
  ffmpeg_video_raw_image
  ffmpeg_video_raw_metadata
  ffmpeg_video_settings
  ffmpeg_video_uninterpreted_data
  )

kwiver_expand_class_list( "${plugin_ffmpeg_classes}" plugin_ffmpeg_headers plugin_ffmpeg_sources )

kwiver_install_headers(
  SUBDIR     arrows/ffmpeg
  ${plugin_ffmpeg_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_ffmpeg_export.h
  NOPATH   SUBDIR     arrows/ffmpeg
  )

if(KWIVER_ENABLE_CUDA)
  set(KWIVER_ENABLE_FFMPEG_CUDA ON CACHE BOOL "Enable use of CUVID/NVENC codecs")
  if(KWIVER_ENABLE_FFMPEG_CUDA)
    find_package(CUDA REQUIRED)
    add_compile_definitions(KWIVER_ENABLE_FFMPEG_CUDA)
    set(FFMPEG_CUDA_LIBRARIES ${CUDA_CUDA_LIBRARY})
    include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
  endif()
endif()

kwiver_add_library( kwiver_algo_ffmpeg
  ${plugin_ffmpeg_headers}
  ${plugin_ffmpeg_sources}
  )

if(KWIVER_ENABLE_ZLIB)
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

if (KWIVER_ENABLE_FFMPEG_CUDA)
  target_include_directories(kwiver_algo_ffmpeg PRIVATE ${CUDA_TOOLKIT_INCLUDE})
endif()

algorithms_create_plugin( kwiver_algo_ffmpeg
  algo/register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
