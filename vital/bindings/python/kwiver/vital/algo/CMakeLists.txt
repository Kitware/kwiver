project(vital_python_algo)

include_directories(${pybind11_INCLUDE_DIR})

set( vital_python_algorithm_headers
      algorithm.h
      image_object_detector.h
   )

set( vital_python_algorithm_sources
      algorithm.cxx
      image_object_detector.cxx
      algorithm_module.cxx
      trampoline/algorithm_trampoline.txx
      trampoline/image_object_detector_trampoline.txx
   )


kwiver_add_python_library(algorithm
                          vital/algo
                          ${vital_python_algorithm_header}
                          ${vital_python_algorithm_sources}
)

target_link_libraries(python-vital.algo-algorithm
  PUBLIC ${PYTHON_LIBRARIES}
          vital
          vital_config
          vital_algo
          vital_python_util
)

kwiver_add_python_library(algorithm_factory
  vital/algo
  algorithm_factory.cxx
)

target_link_libraries(python-vital.algo-algorithm_factory
  PUBLIC ${PYTHON_LIBRARIES}
          vital
          vital_algo
)

if(SKBUILD)
  set_target_properties(python-vital.algo-algorithm
    PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../../lib:\$ORIGIN/"
  )
  set_target_properties(python-vital.algo-algorithm_factory
    PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../../lib:\$ORIGIN/"
  )
endif()

if( NOT SKBUILD )
  kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
                            vital/algo
                            __init__ )
endif()
