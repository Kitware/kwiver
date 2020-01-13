project(vital_python_algo)

include_directories(${pybind11_INCLUDE_DIR})

kwiver_add_python_library(algorithm
  vital/algo
  algorithm.h
  analyze_tracks.h
  associate_detections_to_tracks.h
  bundle_adjust.h
  close_loops.h
  image_object_detector.h
  algorithm.cxx
  analyze_tracks.cxx
  associate_detections_to_tracks.cxx
  bundle_adjust.cxx
  close_loops.cxx
  image_object_detector.cxx
  algorithm_module.cxx
  trampoline/algorithm_trampoline.txx
  trampoline/analyze_tracks_trampoline.txx
  trampoline/close_loops_trampoline.txx
  trampoline/bundle_adjust_trampoline.txx
  trampoline/image_object_detector_trampoline.txx
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

kwiver_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
                                vital/algo
                                __init__)
