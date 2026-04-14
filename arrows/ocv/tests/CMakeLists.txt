project(arrows_test_ocv)

set(CMAKE_FOLDER "Arrows/OCV/Tests")

include(kwiver-test-setup)

set(test_libraries      vital vital_vpm kwiver_algo_ocv)

##############################
# Algorithms OpenCV tests
##############################
kwiver_discover_gtests(ocv algo_config                 LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv bounding_box                LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv descriptor_set              LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv detect_features             LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv detect_heat_map             LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv detect_motion               LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv distortion                  LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv draw_detected_object_set    LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv extract_descriptors         LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv feature_detect_extract      LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv feature_set                 LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv hough_circle_detector       LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv image                       LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv image_CLAHE                 LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv image_histogram_eq          LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv inpaint                     LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(ocv match_features              LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv match_set                   LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv merge_images                LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv refine_detections_write_to_disk
                                                       LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv split_image                 LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv track_features_klt          LIBRARIES ${test_libraries})


# Additional tests that depend on the MVG arrow
if (KWIVER_ENABLE_MVG)
  list(APPEND test_libraries kwiver_algo_mvg)

  kwiver_discover_gtests(ocv analyze_tracks              LIBRARIES ${test_libraries})
  kwiver_discover_gtests(ocv draw_tracks                 LIBRARIES ${test_libraries})
  kwiver_discover_gtests(ocv estimate_fundamental_matrix LIBRARIES ${test_libraries})
  kwiver_discover_gtests(ocv estimate_homography         LIBRARIES ${test_libraries})
  kwiver_discover_gtests(ocv estimate_pnp                LIBRARIES ${test_libraries})
  kwiver_discover_gtests(ocv resection_camera            LIBRARIES ${test_libraries})

endif()
