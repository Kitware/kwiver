# Python cmake, if testing enabled set pip to install test dependencies in venv
if (KWIVER_ENABLE_TESTS AND VENV_CREATED)
    if (PIP_COMMAND)
        python_target_add_command( "pip-install-upgrade"
                                   "${PIP_UPGRADE_COMMAND}"
                                   "Upgrading pip...")
        python_target_add_command( "pip-install-test-deps"
                                   "${PIP_INSTALL_TEST_DEPS_COMMAND}"
                                   "Pip installing Python dependencies..."
                                   "pip-install-upgrade")
    endif()
endif()

if(NOT SKBUILD)
  # TODO depend on a global python target that includes all module files,so it does not run every time
  # Create egg info for the packages mentioned in the entrypoints entry of setup.py
  add_custom_target(install_egg_info ALL
    COMMENT "Install python egg info in the build directory"
    COMMAND ${Python_EXECUTABLE} ${KWIVER_SOURCE_DIR}/setup.py egg_info --egg-base ${KWIVER_BINARY_DIR}/${python_site_packages}
    )

  kwiver_install(
    DIRECTORY ${KWIVER_BINARY_DIR}/${python_site_packages}/kwiver.egg-info
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${python_site_packages}/
  )

endif()

add_subdirectory(kwiver)
