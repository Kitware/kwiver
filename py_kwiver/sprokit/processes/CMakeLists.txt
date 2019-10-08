
kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/kwiver_process.py
  kwiver/sprokit/processes
  kwiver_process )

kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/ApplyDescriptor.py
  kwiver/sprokit/processes
  ApplyDescriptor )

kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/homography_writer.py
  kwiver/sprokit/processes
  homography_writer )

kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/kw_print_number_process.py
  kwiver/sprokit/processes
  kw_print_number_process )

kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/ProcessImage.py
  kwiver/sprokit/processes
  ProcessImage )

kwiver_create_python_init( kwiver/sprokit/processes )
