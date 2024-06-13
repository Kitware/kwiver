include(kwiver-test-setup)

add_test(NAME vtk:applets:estimate_depth
  COMMAND kwiver estimate-depth
            --config "${kwiver_test_data_directory}/config_files/estimate_depth_testing.conf"
            --video-source "${kwiver_test_data_directory}/videos/aphill_240_1fps_crf32.mp4"
            --input-cameras-dir "${kwiver_test_data_directory}/aphill_pipeline_data/krtd"
            --input-landmarks-file "${kwiver_test_data_directory}/aphill_pipeline_data/landmarks.ply")
add_test(NAME vtk:applets:fuse_depth
  COMMAND kwiver fuse-depth
            --config "${kwiver_test_data_directory}/config_files/fuse_depth_testing.conf"
            --input-cameras-dir "${kwiver_test_data_directory}/aphill_pipeline_data/krtd"
            --input-landmarks-file "${kwiver_test_data_directory}/aphill_pipeline_data/landmarks.ply"
            --input-geo-origin-file "${kwiver_test_data_directory}/aphill_pipeline_data/geo_origin.txt"
            --input-depths-dir "${kwiver_test_data_directory}/aphill_pipeline_data/depths")
add_test(NAME vtk:applets:color_mesh
  COMMAND kwiver color-mesh
            --config "${kwiver_test_data_directory}/config_files/color_mesh_testing.conf"
            --input-mesh "${kwiver_test_data_directory}/aphill_pipeline_data/mesh.vtp"
            --video-file "${kwiver_test_data_directory}/videos/aphill_240_1fps_crf32.mp4"
            --cameras-dir "${kwiver_test_data_directory}/aphill_pipeline_data/krtd"
            --input-geo-origin-file "${kwiver_test_data_directory}/aphill_pipeline_data/geo_origin.txt"
            --output-mesh results/colored_mesh.vtp)
