set( stanag_python_headers
     stanag_4607_packet.h
     segments/stanag_4607_segments.h
     segments/stanag_4607_mission_segment.h
     segments/stanag_4607_dwell_segment.h
   )

 set( stanag_python_sources
      stanag_4607_packet.cxx
      stanag_module.cxx
      segments/stanag_4607_segments.cxx
      segments/stanag_4607_mission_segment.cxx
      segments/stanag_4607_dwell_segment.cxx
    )

kwiver_add_python_library( stanag
   arrows/stanag
   ${stanag_python_headers}
   ${stanag_python_sources}
)

target_link_libraries( python-arrows.stanag-stanag
  PRIVATE ${PYTHON_LIBRARIES} pybind11::pybind11
         vital
         kwiver_algo_stanag
)

kwiver_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
                          arrows/stanag
                          __init__)
