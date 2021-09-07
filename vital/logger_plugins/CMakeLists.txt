#
# Plugin modules for vital logger.
#

###
# Build log4cxx plug-in if configured
if (KWIVER_ENABLE_DEP_LOG4CXX)
  set ( log4cxx_lib     ${Log4cxx_LIBRARY} ${ApacheRunTime_LIBRARY} )
  include_directories ( SYSTEM ${Log4cxx_INCLUDE_DIR} ${ApacheRunTime_INCLUDE_DIR} )
  kwiver_add_plugin( vital_log4cxx_logger
                     SOURCES          log4cxx_factory.cxx
                     PRIVATE          ${log4cxx_lib}
                     kwiversys
                     vital_logger
                     SUBDIR           ${kwiver_plugin_logger_subdir}
                     )
  kwiver_install_headers(
    ${CMAKE_CURRENT_BINARY_DIR}/vital_log4cxx_logger_export.h
    NOPATH
    SUBDIR      vital/logger
  )
endif()

###
# Build log4cplus plug-in if configured
if (KWIVER_ENABLE_DEP_LOG4CPLUS)
  kwiver_add_plugin( vital_log4cplus_logger
                     SOURCES          log4cplus_factory.cxx
                     PRIVATE          ${log4cplus_lib}
                     kwiversys
                     vital_logger
                     vital_util
                     log4cplus::log4cplus
                     SUBDIR           ${kwiver_plugin_logger_subdir}
                     )
  kwiver_install_headers(
    ${CMAKE_CURRENT_BINARY_DIR}/vital_log4cplus_logger_export.h
    NOPATH
    SUBDIR      vital/logger
  )
endif()
