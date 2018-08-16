project(vital_python_algo)

include_directories(${PYBIND11_INCLUDE_DIR})

kwiver_add_python_library(algorithm
  vital/algo
  algorithm.cxx)
target_link_libraries(python-vital.algo-algorithm
  PUBLIC ${PYTHON_LIBRARIES}
         vital
         vital_algo)

kwiver_add_python_library(interpolate_track
  vital/algo
  interpolate_track.cxx)
target_link_libraries(python-vital.algo-interpolate_track
  PUBLIC ${PYTHON_LIBRARIES}
         vital
         vital_algo)

kwiver_create_python_init(vital/algo
  algorithm
  interpolate_track
)
