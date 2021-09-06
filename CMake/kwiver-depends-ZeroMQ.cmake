#
# Optionally find and configure ZeroMQ dependency

option( KWIVER_ENABLE_DEP_ZeroMQ
  "Enable ZeroMQ dependent code and plugins"
  OFF
  )

if( KWIVER_ENABLE_DEP_ZeroMQ )
  if(KWIVER_BUILD_SHARED)
    find_package( ZeroMQ REQUIRED )
  else()
    # Zeromq builds both shared static and shared library
    # This allows static library to be found in a static build
    find_library( ZeroMQ_LIBRARY libzmq.a libzmq
                  PATHS ${fletch_ROOT}
                  PATH_SUFFIXES lib
                )
    # Without the use of the `find_package` above, the `ZeroMQ_INCLUDE_DIR`
    # variable does not get set. Setting it here to the known location for ZMQ
    # headers (also covers cppzmq).
    set(ZeroMQ_INCLUDE_DIR "${fletch_ROOT}/include")
  endif()
  include_directories(SYSTEM ${ZeroMQ_INCLUDE_DIR})
endif( KWIVER_ENABLE_DEP_ZeroMQ )
