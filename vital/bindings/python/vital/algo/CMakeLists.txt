project(vital_python_algo)

include_directories(${PYBIND11_INCLUDE_DIR})

kwiver_add_python_library(algorithm
  vital/algo
  algorithm.cxx)
target_link_libraries(python-vital.algo-algorithm
  PUBLIC ${PYTHON_LIBRARIES}
         vital
         vital_vpm
         vital_algo)

kwiver_create_python_init(vital/algo
  algorithm
)
