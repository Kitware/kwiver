
set(SOURCE "main.cpp"
           "how_to_part_01_images.cpp"
           "how_to_part_02_detections.cpp"
           "how_to_part_03_tracking.cpp"
)

set(kwiver_examples_libraries vital_algo
                              kwiversys )

if (fletch_ENABLED_OpenCV)
  list(APPEND kwiver_examples_libraries kwiver_algo_ocv)
endif()

if (fletch_ENABLED_VXL)
  list(APPEND kwiver_examples_libraries kwiver_algo_vxl)
endif()

if(fletch_ENABLED_YAMLCPP)
  add_subdirectory( kpf )

  list(APPEND kwiver_examples_libraries kwiver_algo_kpf kpf_yaml ${YAML_CPP_LIBRARIES})
  if(WIN32)
    set(kwiver_examples_flags YAML_CPP_DLL)
  endif()
endif()

add_executable(kwiver_examples ${SOURCE})
target_link_libraries(kwiver_examples ${kwiver_examples_libraries})
target_compile_definitions(kwiver_examples PRIVATE ${kwiver_examples_flags})
