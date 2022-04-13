include(kwiver-test-setup)

add_test(NAME core:applets:dump_klv
  COMMAND kwiver dump-klv
            --config "${kwiver_test_data_directory}/config_files/dump_klv_testing.conf"
            "${kwiver_test_data_directory}/videos/aphill_klv_10s.ts")
add_test(NAME core:applets:render_mesh
  COMMAND kwiver render-mesh
#            -c "${kwiver_test_data_directory}/config_files/render_mesh_testing.conf"
            "${kwiver_test_data_directory}/aphill_pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.ply"
            "${kwiver_test_data_directory}/aphill_pipeline_data/krtd/aphill_240_1fps_crf32-00001.krtd"
            output_image)
add_test(NAME core:applets:transcode
  COMMAND kwiver transcode
            -c "${kwiver_test_data_directory}/config_files/transcode_testing.conf"
            --input "${kwiver_test_data_directory}/videos/aphill_klv_10s.ts"
            --output transcode_out.ts)

