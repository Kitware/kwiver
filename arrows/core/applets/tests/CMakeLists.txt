include(kwiver-test-setup)

add_test(NAME core:applets:render_mesh
  COMMAND kwiver render-mesh
            -c "${kwiver_test_data_directory}/config_files/render_mesh_testing.conf"
            "${kwiver_test_data_directory}/"
            "${kwiver_test_data_directory}/"
            output_image)
