kwiver_add_executable( demo_macro_magic demo_macro_magic.cxx )
target_link_libraries(
  demo_macro_magic
  PRIVATE   vital
)

kwiver_add_executable( demo_python_impl_call demo_python_impl_call.cxx )
target_link_libraries(
  demo_python_impl_call
  PRIVATE   vital
)

configure_file(
  demo_python_impl_call.py
  ${KWIVER_BINARY_DIR}/bin/demo_python_impl_call.py
  COPYONLY
)
