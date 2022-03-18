include(kwiver-test-setup)

add_test(NAME vtk:applets:fuse_depth
  COMMAND kwiver fuse-depth
            --config "${kwiver_test_data_directory}/config_files/fuse_depth_testing.conf"
            --input-cameras-dir "${kwiver_test_data_directory}/pipeline_data/krtd"
            --input-landmarks-file "${kwiver_test_data_directory}/pipeline_data/landmarks.ply"
            --input-geo-origin-file "${kwiver_test_data_directory}/pipeline_data/geo_origin.txt"
            --input-depths-dir "${kwiver_test_data_directory}/pipeline_data/depths")
