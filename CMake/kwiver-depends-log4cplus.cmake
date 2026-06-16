#
# Optionally find and configure log4cplus
#
if(DEFINED fletch_ENABLED_Log4cplus)
  set(_default ${fletch_ENABLED_Log4cplus})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_LOG4CPLUS
  "Enable log4cplus dependent code for a Vital logger plugin"
  ${_default}
  )
unset(_default)

if (KWIVER_ENABLE_LOG4CPLUS)

  find_package (log4cplus REQUIRED)

else (KWIVER_ENABLE_LOG4CPLUS)

  unset ( log4cplus_DIR         CACHE )
  unset ( log4cplus_FOUND       CACHE )
  unset ( log4cplus_INCLUDE_DIR CACHE )
  unset ( log4cplus_LIBRARY     CACHE )

endif (KWIVER_ENABLE_LOG4CPLUS)
