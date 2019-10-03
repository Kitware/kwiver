project(sprokit_python_sprokit)

add_subdirectory(pipeline)

add_subdirectory(pipeline_util)

if(NOT WIN32)
  add_subdirectory(util)
endif()
