project(sprokit_test_pipeline)

include(kwiver-test-setup)

set(test_libraries
  vital_config vital_vpm
  ${Boost_SYSTEM_LIBRARY}
  sprokit_pipeline
  )

##############################
# Datum tests
##############################
kwiver_discover_gtests(datum test_libraries)

##############################
# Stamp tests
##############################
kwiver_discover_gtests(stamp test_libraries)

##############################
# Edge tests
##############################
set(edge_libraries
  ${test_libraries}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_CHRONO_LIBRARY})

kwiver_discover_gtests(edge edge_libraries)

##############################
# Modules tests
##############################
function (sprokit_add_test_plugin    name    directory)
  kwiver_add_plugin("${name}"
    SUBDIR     ${kwiver_plugin_process_subdir}
    SOURCES    ${ARGN}
    PRIVATE    vital_vpm
               sprokit_pipeline)

  set(output_dir
    "${CMAKE_CURRENT_BINARY_DIR}/${directory}")

  # Forcefully dump the plugin to the current binary directory.
  set_target_properties("${name}"
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
      LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
      ARCHIVE_OUTPUT_DIRECTORY "${output_dir}")
  foreach (config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${config}" upper_config)

    set_target_properties(${name}
      PROPERTIES
        "ARCHIVE_OUTPUT_DIRECTORY_${upper_config}" "${output_dir}/${config}"
        "LIBRARY_OUTPUT_DIRECTORY_${upper_config}" "${output_dir}/${config}"
        "RUNTIME_OUTPUT_DIRECTORY_${upper_config}" "${output_dir}/${config}")
  endforeach ()
endfunction ()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

sprokit_add_test_plugin(processes_test multiple_load
  processes_test.cxx)

sprokit_add_test_plugin(schedulers_test multiple_load
  schedulers_test.cxx)

sprokit_add_test_plugin(not_a_plugin not_a_plugin
  not_a_plugin.cxx)

kwiver_discover_gtests(modules test_libraries test_modules.cxx)

##############################
# Process tests
##############################
kwiver_discover_gtests(process test_libraries test_process.cxx)

##############################
# Process cluster tests
##############################
kwiver_discover_gtests(process_cluster test_libraries test_process_cluster.cxx)

##############################
# Scheduler tests
##############################
kwiver_discover_gtests(scheduler test_libraries test_scheduler.cxx)

##############################
# Pipeline tests
##############################
kwiver_discover_gtests(pipeline test_libraries test_pipeline.cxx)

##############################
# Introspection tests
##############################
sprokit_build_tooled_test(process_introspection test_libraries test_process_introspection.cxx)

set(test_python_path)

if (KWIVER_ENABLE_PYTHON)
  set(python_module_path
    "${sprokit_python_output_path}/${CMAKE_CFG_INTDIR}/${kwiver_python_subdir}")

  set(cur_python_path
    "$ENV{PYTHONPATH}")

  if (cur_python_path)
    set(cur_python_path
      ":${cur_python_path}")
  endif ()

  set(test_python_path
    "${python_module_path}${cur_python_path}")
endif ()

# TODO C++ exceptions get lost in the Python glue code.
#set(sprokit_test_environment "PYTHONPATH=${test_python_path}")
sprokit_add_tooled_test(process_introspection all)
unset(sprokit_test_environment)

##############################
# Process registry tests
##############################
kwiver_discover_gtests(process_registry test_libraries)

##############################
# Scheduler registry tests
##############################
kwiver_discover_gtests(scheduler_registry test_libraries)

##############################
# Running tests
##############################
sprokit_build_tooled_test(run test_libraries)

set(schedulers
  sync
  thread_per_process)

if (KWIVER_ENABLE_PYTHON)
  list(APPEND schedulers
    pythread_per_process)
endif ()

function (sprokit_add_tooled_run_test group instance)
  foreach (scheduler IN LISTS schedulers)
    if (KWIVER_ENABLE_PYTHON)
      if (scheduler STREQUAL "pythread_per_process" OR
          instance STREQUAL "pysimple_pipeline")
        set(sprokit_test_environment "PYTHONPATH=${test_python_path}/${python_sitename}")
      endif ()
    endif ()

    sprokit_add_tooled_test(${group} ${instance}-${scheduler})

    set_tests_properties(test-${group}-${instance}-${scheduler}
      PROPERTIES
        TIMEOUT 5)

    unset(sprokit_test_environment)
  endforeach ()
endfunction ()

sprokit_add_tooled_run_test(run simple_pipeline)

if (KWIVER_ENABLE_PYTHON)
  sprokit_add_tooled_run_test(run pysimple_pipeline)
endif ()

sprokit_add_tooled_run_test(run multiplier_pipeline)
sprokit_add_tooled_run_test(run multiplier_cluster_pipeline)
sprokit_add_tooled_run_test(run frequency_pipeline)
