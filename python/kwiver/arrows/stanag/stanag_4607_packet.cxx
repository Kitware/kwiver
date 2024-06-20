// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_packet.h"

using ptr_t = uint8_t const*;

void
stanag_4607_packet( py::module& m )
{
  py::enum_< kas::stanag_4607_security_classification >( m,
                                                         "stanag_4607_security_classification" )
    .value( "STANAG_4607_SECURITY_CLASS_TOP_SECRET",
            kas::STANAG_4607_SECURITY_CLASS_TOP_SECRET )
    .value( "STANAG_4607_SECURITY_CLASS_SECRET",
            kas::STANAG_4607_SECURITY_CLASS_SECRET )
    .value( "STANAG_4607_SECURITY_CLASS_CONFIDENTIAL",
            kas::STANAG_4607_SECURITY_CLASS_CONFIDENTIAL )
    .value( "STANAG_4607_SECURITY_CLASS_RESTRICTED",
            kas::STANAG_4607_SECURITY_CLASS_RESTRICTED )
    .value( "STANAG_4607_SECURITY_CLASS_UNCLASSIFIED",
            kas::STANAG_4607_SECURITY_CLASS_UNCLASSIFIED )
    .value( "STANAG_4607_SECURITY_CLASS_ENUM_END",
            kas::STANAG_4607_SECURITY_CLASS_ENUM_END )
    .def( "__str__",
          []( kas::stanag_4607_security_classification const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::enum_< kas::stanag_4607_security_code >( m, "stanag_4607_security_code" )
    .value( "STANAG_4607_SECURITY_CODE_NONE",
            kas::STANAG_4607_SECURITY_CODE_NONE )
    .value( "STANAG_4607_SECURITY_CODE_NOCONTRACT",
            kas::STANAG_4607_SECURITY_CODE_NOCONTRACT )
    .value( "STANAG_4607_SECURITY_CODE_ORCON",
            kas::STANAG_4607_SECURITY_CODE_ORCON )
    .value( "STANAG_4607_SECURITY_CODE_PROPIN",
            kas::STANAG_4607_SECURITY_CODE_PROPIN )
    .value( "STANAG_4607_SECURITY_CODE_WNINTEL",
            kas::STANAG_4607_SECURITY_CODE_WNINTEL )
    .value( "STANAG_4607_SECURITY_CODE_NATIONAL_ONLY",
            kas::STANAG_4607_SECURITY_CODE_NATIONAL_ONLY )
    .value( "STANAG_4607_SECURITY_CODE_LIMDIS",
            kas::STANAG_4607_SECURITY_CODE_LIMDIS )
    .value( "STANAG_4607_SECURITY_CODE_FOUO",
            kas::STANAG_4607_SECURITY_CODE_FOUO )
    .value( "STANAG_4607_SECURITY_CODE_EFTO",
            kas::STANAG_4607_SECURITY_CODE_EFTO )
    .value( "STANAG_4607_SECURITY_CODE_LIM_OFF_USE",
            kas::STANAG_4607_SECURITY_CODE_LIM_OFF_USE )
    .value( "STANAG_4607_SECURITY_CODE_NONCOMPARTMENT",
            kas::STANAG_4607_SECURITY_CODE_NONCOMPARTMENT )
    .value( "STANAG_4607_SECURITY_CODE_SPECIAL_CONTROL",
            kas::STANAG_4607_SECURITY_CODE_SPECIAL_CONTROL )
    .value( "STANAG_4607_SECURITY_CODE_SPECIAL_INTEL",
            kas::STANAG_4607_SECURITY_CODE_SPECIAL_INTEL )
    .value( "STANAG_4607_SECURITY_CODE_WARNING_NOTICE",
            kas::STANAG_4607_SECURITY_CODE_WARNING_NOTICE )
    .value( "STANAG_4607_SECURITY_CODE_REL_NATO",
            kas::STANAG_4607_SECURITY_CODE_REL_NATO )
    .value( "STANAG_4607_SECURITY_CODE_REL_4_EYES",
            kas::STANAG_4607_SECURITY_CODE_REL_4_EYES )
    .value( "STANAG_4607_SECURITY_CODE_REL_9_EYES",
            kas::STANAG_4607_SECURITY_CODE_REL_9_EYES )
    .value( "STANAG_4607_SECURITY_CODE_ENUM_END",
            kas::STANAG_4607_SECURITY_CODE_ENUM_END )
    .def( "__str__",
          []( kas::stanag_4607_security_code const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_packet_security >( m,
                                                  "stanag_4607_packet_security" )
    .def( py::init(
            []( kas::stanag_4607_security_classification& classification,
                std::string& class_system,
                kas::stanag_4607_security_code& code){
              return kas::stanag_4607_packet_security{ classification,
                                                       class_system, code };
            } ) )
    .def_readwrite( "classification",
                    &kas::stanag_4607_packet_security::classification )
    .def_readwrite( "class_system",
                    &kas::stanag_4607_packet_security::class_system )
    .def_readwrite( "code", &kas::stanag_4607_packet_security::code )
    .def( "__str__",
          []( kas::stanag_4607_packet_security const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::enum_< kas::stanag_4607_exercise_indicator >( m,
                                                    "stanag_4607_exercise_indicator" )
    .value( "STANAG_4607_EXERCISE_IND_OPERATION_REAL",
            kas::STANAG_4607_EXERCISE_IND_OPERATION_REAL )
    .value( "STANAG_4607_EXERCISE_IND_OPERATION_SIMULATED",
            kas::STANAG_4607_EXERCISE_IND_OPERATION_SIMULATED )
    .value( "STANAG_4607_EXERCISE_IND_OPERATION_SYNTHESIZED",
            kas::STANAG_4607_EXERCISE_IND_OPERATION_SYNTHESIZED )
    .value( "STANAG_4607_EXERCISE_IND_EXERCISE_REAL",
            kas::STANAG_4607_EXERCISE_IND_EXERCISE_REAL )
    .value( "STANAG_4607_EXERCISE_IND_EXERCISE_SIMULATED",
            kas::STANAG_4607_EXERCISE_IND_EXERCISE_SIMULATED )
    .value( "STANAG_4607_EXERCISE_IND_EXERCISE_SYNTHESIZED",
            kas::STANAG_4607_EXERCISE_IND_EXERCISE_SYNTHESIZED )
    .value( "STANAG_4607_EXERCISE_IND_ENUM_END",
            kas::STANAG_4607_EXERCISE_IND_ENUM_END )
    .def( "__str__",
          []( kas::stanag_4607_exercise_indicator const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_packet_header >( m,
                                                "stanag_4607_packet_header" )
    .def( py::init(
            []( std::string& version_id, size_t packet_size,
                std::string& nationality,
                kas::stanag_4607_packet_security& packet_security,
                kas::stanag_4607_exercise_indicator exercise_indicator,
                std::string& platform_id, int& mission_id, int& job_id ){
              return kas::stanag_4607_packet_header{
                version_id, packet_size, nationality, packet_security,
                exercise_indicator, platform_id, mission_id, job_id };
            } ) )
    .def_readwrite( "version_id", &kas::stanag_4607_packet_header::version_id )
    .def_readwrite( "packet_size",
                    &kas::stanag_4607_packet_header::packet_size )
    .def_readwrite( "nationality",
                    &kas::stanag_4607_packet_header::nationality )
    .def_readwrite( "packet_security",
                    &kas::stanag_4607_packet_header::packet_security )
    .def_readwrite( "exercise_indicator",
                    &kas::stanag_4607_packet_header::exercise_indicator )
    .def_readwrite( "platform_id",
                    &kas::stanag_4607_packet_header::platform_id )
    .def_readwrite( "mission_id", &kas::stanag_4607_packet_header::mission_id )
    .def_readwrite( "job_id", &kas::stanag_4607_packet_header::job_id )
    .def( "__str__",
          []( kas::stanag_4607_packet_header const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_packet_header_format  >( m,
                                                        "stanag_4607_packet_header_format" )
    .def( py::init<>() )
    .def_property_readonly_static( "size", [](){
                                     return kas::stanag_4607_packet_header_format::size;
                                   } )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_packet_header_format{}.read( ptr );
          } );

  py::class_< kas::stanag_4607_packet >( m, "stanag_4607_packet" )
    .def_readwrite( "header", &kas::stanag_4607_packet::header )
    .def_readwrite( "segment_headers",
                    &kas::stanag_4607_packet::segment_headers )
    .def_property( "segments",
                   // getter
                   []( kas::stanag_4607_packet& self ){
                     py::list buffer;
                     for( auto segment : self.segments )
                     {
                       std::visit( [ &buffer ]( auto& x ){
                                     buffer.append( x );
                                   }, segment );
                     }
                     return buffer;
                   },
                   // setter
                   []( kas::stanag_4607_packet& self,
                       std::vector< kas::stanag_4607_segments > segments ){
                     self.segments = segments;
                   }
                )
    .def( "__str__",
          []( kas::stanag_4607_packet const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_packet_format >( m,
                                                "stanag_4607_packet_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_packet_format{}.read( ptr );
          } );

  m.def( "read_stanag_4607_data", &kas::read_stanag_4607_data );
  m.def( "read_stanag_4607_file", &kas::read_stanag_4607_file );
}
