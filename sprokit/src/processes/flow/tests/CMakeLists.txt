project(flow_tests)

include(kwiver-test-setup)

set( test_libraries       vital_algo sprokit_pipeline sprokit_tools kwiver_adapter )

#############################
# flow process tests
#############################

kwiver_discover_tests(flow_mux_test             test_libraries test_mux.cxx)
