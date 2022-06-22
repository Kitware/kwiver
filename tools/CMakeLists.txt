kwiver_add_executable( demo_python_impl_call demo_python_impl_call.cxx )
target_link_libraries(
  demo_python_impl_call
  PRIVATE   vital
)
