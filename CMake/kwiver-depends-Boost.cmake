# Required Boost external dependency

if(WIN32)
  set(Boost_USE_STATIC_LIBS TRUE)
  set(Boost_WIN_MODULES chrono)
endif()

if (NOT DEFINED KWIVER_BOOST_VERSION)
	set(KWIVER_BOOST_VERSION 1.54)
endif()

find_package(Boost ${KWIVER_BOOST_VERSION} REQUIRED
  COMPONENTS
    chrono
    date_time
    ${kwiver_boost_python_package}
    filesystem
    program_options
    regex
    system
    thread)

add_definitions(-DBOOST_ALL_NO_LIB)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
####
