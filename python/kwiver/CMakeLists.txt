# Yes, this needs the CMAKE_CURRENT_SOURCE_DIR here.
kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py "" __init__ )

add_subdirectory( vital )
