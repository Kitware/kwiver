project(sprokit_python_schedulers)

set(python_both_arch TRUE)
kwiver_create_python_plugin_init(sprokit/schedulers)

kwiver_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/pythread_per_process_scheduler.py
  sprokit/schedulers
  pythread_per_process)
