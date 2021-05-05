
set(__testable_modnames
  )

# All python filenames_we in the vital.tests module
set(__all_modnames
  __init__
  serialize_json_utils
  ${__testable_modnames}
  )

if (KWIVER_ENABLE_SERIALIZE_JSON)
  list(APPEND __testable_modnames
              test_serialize_json
                                )
endif()

foreach (modname IN LISTS __all_modnames)
  kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/${modname}.py
    arrows/tests
    ${modname})
endforeach()

include(kwiver-setup-tests-python)
foreach (modname IN LISTS __testable_modnames)
    kwiver_add_pytest(${modname} python)
endforeach()
