set( SOURCE
  "main.cpp"
  )

set( stereo_libraries
  kwiver_algo_mvg
  )

add_executable(stereo_example ${SOURCE})
target_link_libraries(stereo_example ${stereo_libraries})

