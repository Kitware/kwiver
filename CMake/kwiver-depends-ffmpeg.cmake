# Optional find and configure FFmpeg dependency

if(DEFINED fletch_ENABLED_FFmpeg)
  set(_default ${fletch_ENABLED_FFmpeg})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_FFMPEG
  "Enable FFmpeg dependent code and plugins (Arrows)"
  ${_default}
  )
unset(_default)

if( KWIVER_ENABLE_FFMPEG )
  find_package( FFMPEG 3.0  REQUIRED )
endif( KWIVER_ENABLE_FFMPEG )
