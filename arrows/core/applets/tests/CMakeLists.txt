include(kwiver-test-setup)

add_test(NAME core:applets:dump_klv
  COMMAND kwiver dump-klv
            --config "${kwiver_test_data_directory}/config_files/dump_klv_testing.conf"
            "${kwiver_test_data_directory}/videos/aphill_klv_10s.ts")
