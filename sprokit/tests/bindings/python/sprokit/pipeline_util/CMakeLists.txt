project(sprokit_test_python_sprokit_pipeline_util)

set(__modnames
  bake
  export_
  load
  )

foreach (group IN LISTS __modnames)
  sprokit_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/test-${group}.py
    sprokit/tests
    test-${group})
endforeach()


if (KWIVER_ENABLE_TESTS)
  foreach (group IN LISTS __modnames)
    sprokit_discover_python_tests(${group} test-${group}.py)
  endforeach()
endif()
