# Should these functions be rectified with the kwiver versions
# in kwiver/CMake/utils/kwiver-utils-python-tests.cmake ?


find_package(PythonInterp ${PYTHON_VERSION} REQUIRED)

# TODO: Use "$<CONFIGURATION>" and remove chdir below when generator
# expressions are supported in test properties.
if (WIN32)
  set(sprokit_test_working_path
    "${sprokit_output_dir}/bin")
endif ()

cmake_dependent_option(SPROKIT_ENABLE_COVERAGE_PYTHON "" ON
  SPROKIT_ENABLE_COVERAGE OFF)

if (SPROKIT_ENABLE_COVERAGE_PYTHON)
  set(sprokit_test_runner
    "${PYTHON_EXECUTABLE}"
    -m trace
    --count
    --coverdir "${sprokit_test_working_path}"
    --ignore-dir="\$prefix"
    --ignore-dir="\$exec_prefix")
else ()
  set(sprokit_test_runner
    "${PYTHON_EXECUTABLE}")
endif ()

###
#
# Configures the python test file to the ctest bin directory
#
# Args:
#     group: the suffix of the python file. Weird. We probably should just
#     use the filename.
#     input: filename of the test .py file (includes the extension)
#
# SeeAlso:
#     sprokit-macro-tests.cmake
#     sprokit-macro-python.cmake
#     sprokit-macro-configure.cmake
#     ../support/test.cmake
#
function (sprokit_build_python_test group input)

  if (CMAKE_CONFIGURATION_TYPES)
    set(sprokit_configure_cmake_args
      "\"-Dconfig=${CMAKE_CFG_INTDIR}/\"")
  endif ()

  set(name test-python-${group})
  set(source "${CMAKE_CURRENT_SOURCE_DIR}/${input}")
  set(dest "${sprokit_test_output_path}/\${config}test-python-${group}")

  if (KWIVER_SYMLINK_PYTHON)
      sprokit_symlink_file(${name} ${source} ${dest} PYTHON_EXECUTABLE)
  else()
      sprokit_configure_file(${name} ${source} ${dest} PYTHON_EXECUTABLE)
  endif()

  # TODO: make tooled python tests work again
  #sprokit_declare_tooled_test(python-${group})
  sprokit_declare_test(python-${group})

endfunction ()


###
# Calls CMake `add_test` function under the hood
function (sprokit_add_python_test group instance)
  #message(STATUS "ADD PYTHON SPROK TEST instance = ${instance}")
  #message(STATUS "instance = ${instance}")
  #message(STATUS "group = ${group}")
  set(python_module_path    "${sprokit_python_output_path}/${sprokit_python_subdir}")
  set(python_chdir          ".")

  if (CMAKE_CONFIGURATION_TYPES)
    set(python_module_path      "${sprokit_python_output_path}/$<CONFIGURATION>/${sprokit_python_subdir}")
    set(python_chdir           "$<CONFIGURATION>")
  endif ()

  # Note: `sprokit_test_runner` is set to the python executable which is
  # implicitly passed down to sprokit_add_tooled_test.


  # NOTE: we are simply re-using the kwiver version of this func instead of
  # redefining it for sprokit. It respects the same vars (as long as we pass
  # test output path), so we should be fine.
  # This runs our test with py.test
  kwiver_add_python_test(${group} ${instance}
    TEST_OUTPUT_PATH ${sprokit_test_output_path}
    PYTHON_MODULE_PATH ${python_module_path}
    )

  # TODO: make tooled python tests work again
  #_kwiver_python_site_package_dir( site_dir )
  # calls sprokit_add_test(${test} ${instance} ${ARGN})
  #sprokit_add_tooled_test(python-${group} ${instance}
    #"${python_chdir}" "${python_module_path}/${site_dir}" ${ARGN})
endfunction ()


###
#
# Searches test .py files for functions that begin with "test" and creates a
# separate `ctest` for each. Ideally we would just map the output from
# something like `py.test` to `ctest` instead.
#
# Arg:
#     group: the test is registered with this ctests group
#     file: filename of the test .py file (includes the extension)
#
# SeeAlso:
#     kwiver/CMake/utils/kwiver-utils-python-tests.cmake - defines kwiver_discover_python_tests
#     kwiver/sprokit/tests/bindings/python/sprokit/pipeline/CMakeLists.txt - uses this function
#
function (sprokit_discover_python_tests group file)
  set(properties)

  set(group "${group}.py")

  # Register the python file as a test script
  sprokit_build_python_test("${group}" "${file}")

  # Define a python test for each testable function / method
  set(py_fpath "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  parse_python_testables("${py_fpath}" _testables)

  foreach (test_name IN LISTS _testables)
    sprokit_add_python_test("${group}" "${test_name}" ${ARGN})
  endforeach()

endfunction ()
