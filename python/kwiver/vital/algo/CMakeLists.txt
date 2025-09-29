set( THIS_MODULE vital/algo )

kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py "${THIS_MODULE}" __init__ )

################################################################################

# simple macro to add class.h, class.cxx and class_trampoline.txx to a list for
# each class in class_list
macro(kwiver_expand_pybind_class_list class_list  sources_var)
  foreach(class IN ITEMS ${class_list})
    list(APPEND ${sources_var} "${CMAKE_CURRENT_BINARY_DIR}/${class}.h")
    list(APPEND ${sources_var} "${CMAKE_CURRENT_BINARY_DIR}/${class}.cxx")
    list(APPEND ${sources_var} "${CMAKE_CURRENT_BINARY_DIR}/${class}_trampoline.txx")
  endforeach()
endmacro()

macro(kwiver_generate_dependencies class_list sources_var)
  foreach(class IN ITEMS ${class_list})
    list(APPEND ${sources_var} "${KWIVER_SOURCE_DIR}/vital/algo/${class}.h")
  endforeach()
endmacro()


set( classes
  activity_detector
  analyze_tracks
  associate_detections_to_tracks
  bundle_adjust
  buffered_metadata_filter
  close_loops
  compute_association_matrix
  compute_depth
  compute_ref_homography
  compute_stereo_depth_map
  compute_track_descriptors
  convert_image
  data_serializer
  detect_features
  detect_motion
  detected_object_filter
  detected_object_set_input
  detected_object_set_output
  draw_detected_object_set
  draw_tracks
  dynamic_configuration
  estimate_canonical_transform
  estimate_essential_matrix
  estimate_fundamental_matrix
  estimate_homography
  estimate_pnp
  estimate_similarity_transform
  extract_descriptors
  feature_descriptor_io
  filter_features
  filter_tracks
  image_filter
  image_io
  image_object_detector
  initialize_cameras_landmarks
  initialize_object_tracks
  integrate_depth_maps
  interpolate_track
  keyframe_selection
  match_descriptor_sets
  match_features
  merge_images
  metadata_map_io
  optimize_cameras
  read_object_track_set
  read_track_descriptor_set
  refine_detections
  resection_camera
  split_image
  track_features
  train_detector
  transform_2d_io
  triangulate_landmarks
  uuid_factory
  uv_unwrap_mesh
  video_input
  write_object_track_set
  write_track_descriptor_set
)

# generate configuration file for the wrapper generator
set(KWIVER_PYBIND11_ALGO "${classes}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.ini.in  ${CMAKE_CURRENT_BINARY_DIR}/config.ini.in @ONLY)
# due to https://github.com/CastXML/pygccxml/issues/180 we need to run castxml externally
if (MSVC)
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/config.ini.in
"run_external_castxml=1
compiler_path=${CMAKE_CXX_COMPILER}
")
endif ()
file(GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config.ini
  INPUT  ${CMAKE_CURRENT_BINARY_DIR}/config.ini.in)


# files to generate
set(algo_sources)
kwiver_expand_pybind_class_list("${classes}" algo_sources)
# headers the command depends on
set(vital_algo_dependencies)
kwiver_generate_dependencies("${classes}" vital_algo_dependencies)

# we generate all wrappings at once instead of per file since the startup cost
# of the C++ parser is high
add_custom_command(OUTPUT ${algo_sources}
  COMMAND  ${Python_EXECUTABLE}
          "${KWIVER_SOURCE_DIR}/scripts/cpp_to_pybind11.py"
          -c "${CMAKE_CURRENT_BINARY_DIR}/config.ini"
  DEPENDS ${vital_algo_dependencies}
          "${CMAKE_CURRENT_BINARY_DIR}/config.ini"
          "${KWIVER_SOURCE_DIR}/scripts/cpp_to_pybind11.py"
  COMMENT "Generating pybind11 wrappings of algorithm classes"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  VERBATIM
)

# generate algorithm_module.cxx
#------------------------------------------------------------------
set(_kwiver_python_algo_all_modules_includes)
foreach (_kwiver_python_module IN LISTS classes)
  string(APPEND _kwiver_python_algo_all_modules_includes "#include <python/kwiver/vital/algo/${_kwiver_python_module}.h>\n")
endforeach ()

set(_kwiver_python_algo_all_modules_load_algos)
foreach (_kwiver_python_module IN LISTS classes)
  string(APPEND _kwiver_python_algo_all_modules_load_algos "   ${_kwiver_python_module}(m);\n")
endforeach ()

set(_kwiver_python_algo_all_modules_content)
string(CONCAT _kwiver_python_algo_all_modules_content
"#include <pybind11/pybind11.h>\n"
"#include <python/kwiver/vital/algo/algorithm.h>\n"
"#include <python/kwiver/vital/algo/algorithm_capabilities.h>\n"
"${_kwiver_python_algo_all_modules_includes}"
"namespace kwiver::vital::python {\n"
"PYBIND11_MODULE(algos,m)\n"
"{\n"
"   algorithm(m);\n"
"   algorithm_capabilities(m);\n"
"${_kwiver_python_algo_all_modules_load_algos}"
"}\n"
"} // namespace \n"
)

file(GENERATE
     OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/algorithm_module.cxx"
     CONTENT "${_kwiver_python_algo_all_modules_content}")

#------------------------------------------------------------------


set(module_sources
  algorithm.h
  algorithm.cxx
  algorithm_capabilities.cxx
  algorithm_trampoline.txx
  "${CMAKE_CURRENT_BINARY_DIR}/algorithm_module.cxx"
  ${algo_sources}
)

# Python Module Library: _algos
kwiver_add_python_library(
  algos
  "${THIS_MODULE}"
  SOURCES ${module_sources}
  PRIVATE pybind11::pybind11
          vital
          vital_config
          vital_algo
)
