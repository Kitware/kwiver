
if (KWIVER_ENABLE_OPENCV AND KWIVER_ENABLE_VXL)
  set(sources main.cxx
    how_to_part_01_images.cxx
    how_to_part_02_detections.cxx
    )

  set(kwiver_examples_libraries vital_algo
    kwiver_algo_ocv
    kwiver_algo_vxl
    kwiversys )

  if( KWIVER_ENABLE_FFMPEG )
    add_subdirectory( video )
    list(APPEND kwiver_examples_libraries kwiver_algo_ffmpeg)
  endif()

  if(KWIVER_ENABLE_KPF)
    add_subdirectory( kpf )

    list(APPEND kwiver_examples_libraries kwiver_algo_kpf kpf_yaml ${YAML_CPP_LIBRARIES})
    # yaml-cpp static in fletch - YAML_CPP_DLL must not be defined.
    # if(WIN32)
    #   set(kwiver_examples_flags YAML_CPP_DLL)
    # endif()
  endif()


  add_executable(kwiver_examples ${sources})
  target_link_libraries(kwiver_examples ${kwiver_examples_libraries})
  target_compile_definitions(kwiver_examples PRIVATE ${kwiver_examples_flags})
else()
  message(SEND_ERROR " Building examples requires enabling VXL and OpenCV")
endif()
