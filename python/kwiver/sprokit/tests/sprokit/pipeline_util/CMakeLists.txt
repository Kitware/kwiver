set(kwiver_test_pipelines_directory
  "${kwiver_test_data_directory}/pipelines")

set(__modnames
  bake
  export_
  load
  )

# For now this is always enabled (because we are in the test-dir)
# but the goal is to move python tests into the python source dir soon
if (KWIVER_ENABLE_TESTS)
  include(kwiver-test-setup)
  foreach (group IN LISTS __modnames)
    kwiver_discover_python_tests(${group} test-${group}.py
      ${sprokit_test_pipelines_directory})
  endforeach()
endif()
