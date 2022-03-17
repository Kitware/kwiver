include(kwiver-test-setup)

add_test(NAME core:applets:dump-klv
  COMMAND kwiver dump-klv
            --config "${kwiver_test_data_directory}/dump_klv_testing.conf"
            "${kwiver_test_data_directory}/aphill_klv_10s.mp4")
add_test(NAME core:applets:render_mesh
  COMMAND kwiver render-mesh
            -c "${kwiver_test_data_directory}/render_mesh_testing.conf"
            --mesh "${kwiver_test_data_directory}/"
            --camera "${kwiver_test_data_directory}/"
            --output "${kwiver_test_data_directory}/")
