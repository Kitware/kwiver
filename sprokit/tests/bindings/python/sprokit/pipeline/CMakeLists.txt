project(sprokit_test_python_sprokit_pipeline)

# Simple testables require no pytest paramatarization
set(__simple_testable_modnames
  config
  datum
  edge
  modules
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

set(__all_modnames
  run
  ${__simple_testable_modnames}
  )

# Add files to sprokit.tests module to mimic a standard python source tree
foreach (group IN LISTS __all_modnames)
  sprokit_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/test-${group}.py
    sprokit/tests
    test-${group})
endforeach()


###
# Setup simple tests with no parameters

# KWIVER_ENABLE_TESTS is always true for now (because we are in the test
# subdir), but the python tests should be moved to the python source dir.
if (KWIVER_ENABLE_TESTS)
  #include(kwiver-test-setup)
  # Register individual tests with ctest
  foreach (group IN LISTS __simple_testable_modnames)
    # SeeAlso:
    #     # defines sprokit_discover_python_tests
    #     sprokit/conf/sprokit-macro-python-tests.cmake
    sprokit_discover_python_tests(${group} test-${group}.py)
  endforeach()
endif()


###
# Setup tests that need py.test paramatarization

set(_valid_schedulers
  pythread_per_process
  sync
  thread_per_process)

set(_instances
  python_to_python
  cpp_to_python
  python_to_cpp
  python_via_cpp
)

set(group run)

sprokit_build_python_test(${group}.py test-${group}.py)


foreach (instance IN LISTS _instances)
  foreach (scheduler IN LISTS _valid_schedulers)
    set(pytest_node_suffix "test_${instance}[${scheduler}]")
    sprokit_add_python_test(${group}.py "${pytest_node_suffix}")
    set_tests_properties(test-python-${group}.py-${pytest_node_suffix}
      PROPERTIES
        TIMEOUT 5)
  endforeach ()
endforeach ()
