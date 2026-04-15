# Build / Install plugin containing DBoW2 algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/DBoW2")

set( plugin_dbow2_headers
  DBoW2.h
  FClass.h
  TemplatedDatabase.h
  TemplatedVocabulary.h
  )
set( plugin_dbow2_sources )
set( plugin_dbow2_classes
  algo/match_descriptor_sets
  BowVector
  FeatureVector
  FORB
  QueryResults
  Random
  ScoringObject
  Timestamp
  )

kwiver_expand_class_list( "${plugin_dbow2_classes}" plugin_dbow2_headers plugin_dbow2_sources )

kwiver_install_headers(
  SUBDIR     arrows/dbow2
  ${plugin_dbow2_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_dbow2_export.h
  NOPATH   SUBDIR     arrows/dbow2
  )


kwiver_add_library( kwiver_algo_dbow2
  ${plugin_dbow2_headers}
  ${plugin_dbow2_sources}
  )

target_link_libraries( kwiver_algo_dbow2
  PUBLIC               vital_algo
                       ${OpenCV_LIBS}
  PRIVATE              kwiversys
  )

algorithms_create_plugin( kwiver_algo_dbow2
  register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
