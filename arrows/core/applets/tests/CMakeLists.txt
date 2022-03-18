include(kwiver-test-setup)

add_test(NAME core:applets:render_mesh
  COMMAND kwiver render-mesh
#            -c "${kwiver_test_data_directory}/config_files/render_mesh_testing.conf"
            "${kwiver_test_data_directory}/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.ply"
            "${kwiver_test_data_directory}/pipeline_data/krtd/aphill_240_1fps_crf32-00001.krtd"
            output_image)
