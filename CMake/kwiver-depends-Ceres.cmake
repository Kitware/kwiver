# Optionally find and configure Ceres dependency

if(DEFINED fletch_ENABLED_Ceres)
  set(_default ${fletch_ENABLED_Ceres})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_CERES
  "Enable Ceres dependent code and plugins (Arrows)"
  ${_default}
  )
unset(_default)

if( KWIVER_ENABLE_CERES )
  find_package( Ceres REQUIRED )
  if(NOT CERES_INCLUDE_DIRS)
    # Ceres v2 doesn't define CERES_INCLUDE_DIRS, but the ceres algo plugin still relies on it
    get_target_property(CERES_INCLUDE_DIRS Ceres::ceres INTERFACE_INCLUDE_DIRECTORIES)
  endif()
  include_directories( SYSTEM ${CERES_INCLUDE_DIRS} )
endif()
