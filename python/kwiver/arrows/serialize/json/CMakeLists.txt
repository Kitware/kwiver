set( json_python_headers
     serialize_bounding_box.h
     serialize_detected_object.h
     serialize_detected_object_type.h
   )

set( json_python_sources
     serialize_utils.txx
     serialize_bounding_box.cxx
     serialize_detected_object.cxx
     serialize_detected_object_type.cxx
     serialize_module.cxx
   )

kwiver_add_python_library( json
   arrows/serialize/json
   ${json_python_headers}
   ${json_python_sources}
)

target_link_libraries( python-arrows.serialize.json-json
  PUBLIC ${PYTHON_LIBRARIES}
         vital
         kwiver_serialize_json
)

kwiver_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
                                arrows/serialize/json
                                __init__)
