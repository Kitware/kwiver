project(sprokit_test_python_sprokit_pipeline)

# Simple testables require no pytest paramatarization
set(__simple_testable_modnames
  datum
  edge
  pipeline
  process
  process_cluster
  process_registry
  scheduler
  scheduler_registry
  stamp
  utils
  version
  )


###
# Setup simple tests with no parameters

# KWIVER_ENABLE_TESTS is always true for now (because we are in the test
# subdir), but the python tests should be moved to the python source dir.
if (KWIVER_ENABLE_TESTS and PYTEST_RUNNER)

# Register individual tests with ctest
  foreach (group IN LISTS __simple_testable_modnames)
    # SeeAlso:
    #     # defines sprokit_discover_python_tests
    #     sprokit/conf/sprokit-macro-python-tests.cmake
    kwiver_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/${group}.py sprokit/tests/sprokit/pipeline ${group})
  endforeach()

  include(kwiver-setup-tests-python)

  foreach (group in LISTS __simple_testable_modnames)
    kwiver_add_pytest(${group} python)
  endforeach()
endif()


###
# Setup tests with parameters

set(schedulers
  pythread_per_process
  sync
  thread_per_process)

kwiver_build_python_test(run test-run.py)
function (kwiver_add_python_run_test group instance)
  foreach (scheduler IN LISTS schedulers)
    kwiver_add_python_test(${group} ${instance}-${scheduler})

    set_tests_properties(test-python-${group}-${instance}-${scheduler}
      PROPERTIES
        TIMEOUT 5)
  endforeach ()
endfunction ()

kwiver_add_python_run_test(run python_to_python)
kwiver_add_python_run_test(run cpp_to_python)
kwiver_add_python_run_test(run python_to_cpp)
kwiver_add_python_run_test(run python_via_cpp)
