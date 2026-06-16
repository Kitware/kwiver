# Optionally find and configure PROJ dependency

if(DEFINED fletch_ENABLED_PROJ)
  set(_default ${fletch_ENABLED_PROJ})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_PROJ
  "Enable PROJ dependent code and plugins (Arrows)"
  ${_default}
  )
unset(_default)

if( KWIVER_ENABLE_PROJ )
  find_package( PROJ REQUIRED )
endif( KWIVER_ENABLE_PROJ )
