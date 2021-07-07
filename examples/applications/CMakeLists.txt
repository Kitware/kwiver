set(application_files
  TeleSculptorCLI.py)

# Ensure target directory exists
set(out_dir "${EXAMPLE_DIR}/applications")
add_custom_command(
  OUTPUT "${out_dir}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${out_dir}"
  )

# Set up meta-target
add_custom_target(kwiver-applications ALL DEPENDS "${out_dir}")

# Add targets to copy config files to the build area
foreach(file ${application_files})
  get_filename_component(basename ${file} NAME_WE)
  get_filename_component(filename ${file} NAME)
  set(target ${basename})
  kwiver_configure_file(${target}
    "${CMAKE_CURRENT_SOURCE_DIR}/${filename}"
    "${out_dir}/${filename}"
    )
  add_dependencies(kwiver-applications configure-${target})
endforeach()

# Install config and model files
kwiver_install(
  FILES ${application_files}
  COMPONENT runtime
  DESTINATION ${out_dir}
  )