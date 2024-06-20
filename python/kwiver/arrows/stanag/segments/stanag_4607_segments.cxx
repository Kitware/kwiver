// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_segments.h"

using ptr_t = uint8_t const*;

void
stanag_4607_segments( py::module& m )
{
  py::enum_< kas::stanag_4607_segment_type >( m, "stanag_4607_segment_type" )
    .value( "STANAG_4607_SEGMENT_TYPE_MISSION",
            kas::STANAG_4607_SEGMENT_TYPE_MISSION )
    .value( "STANAG_4607_SEGMENT_TYPE_DWELL",
            kas::STANAG_4607_SEGMENT_TYPE_DWELL )
    .value( "STANAG_4607_SEGMENT_TYPE_HRR", kas::STANAG_4607_SEGMENT_TYPE_HRR )
    .value( "STANAG_4607_SEGMENT_TYPE_JOB_DEFINITION",
            kas::STANAG_4607_SEGMENT_TYPE_JOB_DEFINITION )
    .value( "STANAG_4607_SEGMENT_TYPE_FREE_TEXT",
            kas::STANAG_4607_SEGMENT_TYPE_FREE_TEXT )
    .value( "STANAG_4607_SEGMENT_TYPE_LOW_REFLECTIVITY_INDEX",
            kas::STANAG_4607_SEGMENT_TYPE_LOW_REFLECTIVITY_INDEX )
    .value( "STANAG_4607_SEGMENT_TYPE_GROUP",
            kas::STANAG_4607_SEGMENT_TYPE_GROUP )
    .value( "STANAG_4607_SEGMENT_TYPE_ATTACHED_TARGET",
            kas::STANAG_4607_SEGMENT_TYPE_ATTACHED_TARGET )
    .value( "STANAG_4607_SEGMENT_TYPE_TEST_AND_STATUS",
            kas::STANAG_4607_SEGMENT_TYPE_TEST_AND_STATUS )
    .value( "STANAG_4607_SEGMENT_TYPE_SYSTEM_SPECIFIC",
            kas::STANAG_4607_SEGMENT_TYPE_SYSTEM_SPECIFIC )
    .value( "STANAG_4607_SEGMENT_TYPE_PROCESSING_HISTORY",
            kas::STANAG_4607_SEGMENT_TYPE_PROCESSING_HISTORY )
    .value( "STANAG_4607_SEGMENT_TYPE_PLATFORM_LOCATION",
            kas::STANAG_4607_SEGMENT_TYPE_PLATFORM_LOCATION )
    .value( "STANAG_4607_SEGMENT_TYPE_JOB_REQUEST",
            kas::STANAG_4607_SEGMENT_TYPE_JOB_REQUEST )
    .value( "STANAG_4607_SEGMENT_TYPE_JOB_ACKNOWLEDGE",
            kas::STANAG_4607_SEGMENT_TYPE_JOB_ACKNOWLEDGE )
    .value( "STANAG_4607_SEGMENT_TYPE_ENUM_END",
            kas::STANAG_4607_SEGMENT_TYPE_ENUM_END )
    .def( "__str__",
          []( kas::stanag_4607_segment_type const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_segment_header >( m,
                                                 "stanag_4607_segment_header" )
    .def( py::init(
            []( kas::stanag_4607_segment_type& segment_type,
                size_t& segment_size ){
              return kas::stanag_4607_segment_header{ segment_type,
                                                      segment_size };
            } )
          )
    .def_readwrite( "segment_type",
                    &kas::stanag_4607_segment_header::segment_type )
    .def_readwrite( "segment_size",
                    &kas::stanag_4607_segment_header::segment_size )
    .def( "__str__",
          []( kas::stanag_4607_segment_header const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_segment_header_format >( m,
                                                        "stanag_4607_segment_header_format" )
    .def( py::init<>() )
    .def_property_readonly_static( "size", [](){
                                     return kas::stanag_4607_segment_header_format::size;
                                   } )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_segment_header_format{}.read( ptr );
          } );
}
