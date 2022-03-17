include(kwiver-test-setup)

add_test(NAME core:applets:dump_klv
  COMMAND kwiver dump-klv
            --config "${kwiver_test_data_directory}/dump_klv_testing.conf"
            "${kwiver_test_data_directory}/aphill_klv_10s.ts")
add_test(NAME core:applets:render_mesh
  COMMAND kwiver render-mesh
            -c "${kwiver_test_data_directory}/render_mesh_testing.conf"
            "${kwiver_test_data_directory}/"
            "${kwiver_test_data_directory}/"
            output_image)
