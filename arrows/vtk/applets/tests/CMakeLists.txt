include(kwiver-test-setup)

add_test(NAME vtk:applets:color_mesh
  COMMAND kwiver color-mesh
            --config "${kwiver_test_data_directory}/config_files/color_mesh_testing.conf"
            --input-mesh "${kwiver_test_data_directory}/pipeline_data/mesh.vtp"
            --video-file "${kwiver_test_data_directory}/videos/aphill_240_1fps_crf32.mp4"
            --cameras-dir "${kwiver_test_data_directory}/pipeline_data/krtd"
            --input-geo-origin-file "${kwiver_test_data_directory}/pipeline_data/geo_origin.txt"
            --output-mesh results/colored_mesh.vtp)
