project(sprokit_conf)

if (KWIVER_ENABLE_TESTS)
  # Configure the warning and code coverage suppression file
  kwiver_configure_file(CTestCustom.cmake
    "${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in"
    "${sprokit_binary_dir}/CTestCustom.cmake")
endif ()

if (SPROKIT_ENABLE_CDASH)
  find_program(GCOV_EXECUTABLE gcov)
  execute_process(
    COMMAND         hostname
    OUTPUT_VARIABLE default_site_name
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "^([^.]+)(\\..*)?$" "\\1" short_default_site_name "${default_site_name}")
  set(CDASH_SITE_NAME "${short_default_site_name}"
    CACHE STRING "The hostname to submit to the dashboard with")
  set(CDASH_BUILD_NAME ""
    CACHE STRING "String describing the build (\"regex\": 'os-os_version-arch-buildtool-compiler-compiler_version(-dep-depversion)*(-feature)?')")

  # Configure dashboard script
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/dashboard.ctest.in"
    "${sprokit_binary_dir}/dashboard.ctest"
    @ONLY)
endif ()
