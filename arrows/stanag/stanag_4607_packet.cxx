// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_packet.h"

#include <iostream>
#include <fstream>
#include <sstream>

namespace kwiver {

namespace arrows {

namespace stanag {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_security_classification const& value )
{
  std::map< stanag_4607_security_classification, std::string > strings
  {
    { STANAG_4607_SECURITY_CLASS_TOP_SECRET, "TOP SECRET" },
    { STANAG_4607_SECURITY_CLASS_SECRET, "SECRET" },
    { STANAG_4607_SECURITY_CLASS_CONFIDENTIAL, "CONFIDENTIAL" },
    { STANAG_4607_SECURITY_CLASS_RESTRICTED, "RESTRICTED" },
    { STANAG_4607_SECURITY_CLASS_UNCLASSIFIED, "UNCLASSIFIED" },
    { STANAG_4607_SECURITY_CLASS_ENUM_END, "Unknown Security Classification" }
  };

  os << strings[ std::min( value, STANAG_4607_SECURITY_CLASS_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_security_code const& value )
{
  std::map< stanag_4607_security_code, std::string > strings
  {
    { STANAG_4607_SECURITY_CODE_NONE, "NONE (NO-STATEMENT VALUE)" },
    { STANAG_4607_SECURITY_CODE_NOCONTRACT, "NO CONTRACT" },
    { STANAG_4607_SECURITY_CODE_ORCON, "ORCON" },
    { STANAG_4607_SECURITY_CODE_PROPIN, "PROPIN" },
    { STANAG_4607_SECURITY_CODE_WNINTEL, "WNINTEL" },
    { STANAG_4607_SECURITY_CODE_NATIONAL_ONLY, "NATIONAL ONLY" },
    { STANAG_4607_SECURITY_CODE_LIMDIS, "LIMDIS" },
    { STANAG_4607_SECURITY_CODE_FOUO, "FOUO" },
    { STANAG_4607_SECURITY_CODE_EFTO, "EFTO" },
    { STANAG_4607_SECURITY_CODE_LIM_OFF_USE, "LIM OFF USE (UNCLAS)" },
    { STANAG_4607_SECURITY_CODE_NONCOMPARTMENT, "NONCOMPARTMENT" },
    { STANAG_4607_SECURITY_CODE_SPECIAL_CONTROL, "SPECIAL CONTROL" },
    { STANAG_4607_SECURITY_CODE_SPECIAL_INTEL, "SPECIAL INTEL" },
    { STANAG_4607_SECURITY_CODE_WARNING_NOTICE,
        "WARNING NOTICE – SECURITY CLASSIFICATION IS BASED ON THE FACT "
        "OF EXISTENCE AND AVAIL OF THIS DATA" },
    { STANAG_4607_SECURITY_CODE_REL_NATO, "REL NATO (BEL, BGR, CAN, CZE, DNK, "
        "EST, FRA, DEU, GRC, HUN, ISL, ITA, LVA, LTU, LUX,NLD, NOR, POL, PRT, "
        "ROU, SVK, SVN, ESP, TUR, GBR, USA)" },
    { STANAG_4607_SECURITY_CODE_REL_4_EYES,
        "REL 4-EYES (AUS, CAN, GBR, USA)" },
    { STANAG_4607_SECURITY_CODE_REL_9_EYES,
        "REL 9-EYES (CAN, FRA, DEU, ITA, NLD, NOR, ESP, GBR, USA)" },
    { STANAG_4607_SECURITY_CODE_ENUM_END, "Unknown Security Code" } };

  os << strings[ std::min( value, STANAG_4607_SECURITY_CODE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_packet_security const& value )
{
  return os << "{ "
            << "Classification: " << value.classification << ", "
            << "Class. System: " << value.class_system << ", "
            << "Code: " << value.code
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
  stanag_4607_packet_security,
  &stanag_4607_packet_security::classification,
  &stanag_4607_packet_security::class_system,
  &stanag_4607_packet_security::code
)

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_exercise_indicator const& value )
{
  std::map<stanag_4607_exercise_indicator, std::string> strings
  { { STANAG_4607_EXERCISE_IND_OPERATION_REAL, "Operation, Real Data" },
    { STANAG_4607_EXERCISE_IND_OPERATION_SIMULATED,
        "Operation, Simulated Data" },
    { STANAG_4607_EXERCISE_IND_OPERATION_SYNTHESIZED,
        "Operation, Synthesized Data" },
    { STANAG_4607_EXERCISE_IND_EXERCISE_REAL, "Exercise, Real Data" },
    { STANAG_4607_EXERCISE_IND_EXERCISE_SIMULATED,
        "Exercise, Simulated Data" },
    { STANAG_4607_EXERCISE_IND_EXERCISE_SYNTHESIZED,
        "Exercise, Synthesized Data" },
    { STANAG_4607_EXERCISE_IND_ENUM_END, "Unknown Exercise Indicator" } };

  os << strings[ std::min( value, STANAG_4607_EXERCISE_IND_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_packet_header const& value )
{
  return os << "{ "
            << "Version ID: " << value.version_id << ", "
            << "Packet Size: " << value.packet_size << ", "
            << "Nationality: " << value.nationality << ", "
            << "Packet Security: " << value.packet_security << ", "
            << "Exercise Indicator: " << value.exercise_indicator << ", "
            << "Platform ID: " << value.platform_id << ", "
            << "Mission ID: " << value.mission_id << ", "
            << "Job ID: " << value.job_id
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
  stanag_4607_packet_header,
  &stanag_4607_packet_header::version_id,
  &stanag_4607_packet_header::packet_size,
  &stanag_4607_packet_header::nationality,
  &stanag_4607_packet_header::packet_security,
  &stanag_4607_packet_header::exercise_indicator,
  &stanag_4607_packet_header::platform_id,
  &stanag_4607_packet_header::mission_id,
  &stanag_4607_packet_header::job_id
)

// ----------------------------------------------------------------------------
stanag_4607_packet_header_format
::stanag_4607_packet_header_format()
{}

// ----------------------------------------------------------------------------
stanag_4607_packet_header
stanag_4607_packet_header_format
::read( ptr_t& ptr ) const
{
  stanag_4607_packet_header result;

  result.version_id = klv::klv_read_string( ptr, (size_t)2 );

  result.packet_size = klv::klv_read_int< size_t >( ptr, (size_t)4 );

  result.nationality = klv::klv_read_string( ptr, (size_t)2 );

  result.packet_security.classification =
    static_cast< stanag_4607_security_classification >(
        klv::klv_read_int< uint64_t >( ptr, (size_t)1 ));
  result.packet_security.class_system = klv::klv_read_string( ptr,
                                                              (size_t)2 );
  result.packet_security.code = static_cast< stanag_4607_security_code >(
      klv::klv_read_int< uint64_t >( ptr, (size_t)2 ));

  result.exercise_indicator = static_cast< stanag_4607_exercise_indicator >(
      klv::klv_read_int< uint64_t >( ptr, (size_t)1 ));

  result.platform_id = trim_whitespace( klv::klv_read_string( ptr,
                                                              (size_t)10 ) );

  result.mission_id = klv::klv_read_int< int >( ptr, (size_t)4 );

  result.job_id = klv::klv_read_int< int >( ptr, (size_t)4 );

  return result;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_packet const& value )
{
    os << "{ "
       << "Packet Header: " << value.header;

    auto num_segments = value.segment_headers.size();
    if( num_segments == 0 )
    {
      return os << ", " << "Segment Header: (empty)" << ", "
                << "(No segments)";
    }
    for ( size_t i=0; i<num_segments; i++ )
    {
      auto segment_header = value.segment_headers[i];

      os  << ", " << "Segment Header: " << segment_header << ", "
          << stanag_4607_segment_type_traits_lookup_table()
              .by_type( segment_header.segment_type ).name()
          << ": ";

      os << std::visit([](auto s)
      {
        std::ostringstream stream;
        stream << s;

        return stream.str();
      }, value.segments[i]);
    }

    os << " }";
    return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, std::vector< stanag_4607_packet > const& value )
{
    if ( value.empty() )
    {
      return os << "(empty)";
    }
    for( auto v : value )
    {
        os << v << std::endl;
        os << std::endl;
    }
    return os;
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
    stanag_4607_packet,
    &stanag_4607_packet::header,
    &stanag_4607_packet::segment_headers,
    &stanag_4607_packet::segments
)

// ----------------------------------------------------------------------------
stanag_4607_packet_format
::stanag_4607_packet_format()
{}

// ----------------------------------------------------------------------------
stanag_4607_packet
stanag_4607_packet_format
::read( ptr_t& ptr ) const
{
    size_t bytes_read_in_packet = 0;

    // Read packet header
    stanag_4607_packet_header_format packet_header;

    stanag_4607_packet_header packet_header_data =
        packet_header.read( ptr );

    bytes_read_in_packet += packet_header.size;
    auto packet_size = packet_header_data.packet_size;

    std::vector< stanag_4607_segment_header > segment_headers;
    std::vector< stanag_4607_segments > segments;
    auto it = segments.begin();

    while( bytes_read_in_packet < packet_size )
    {
        // Read segment header
        stanag_4607_segment_header_format segment_header;
        auto segment_header_data = segment_header.read( ptr );
        segment_headers.push_back( segment_header_data );

        // Determine segment type + size from segment header
        stanag_4607_segment_type type = segment_header_data.segment_type;
        auto const& format = stanag_4607_segment_type_traits_lookup_table()
                             .by_type( type ).format();
        size_t segment_size = segment_header_data.segment_size;

        // Read message segment
        stanag_4607_segments message;
        if( typeid(format) == typeid(stanag_4607_mission_segment_format) )
        {
          message = stanag_4607_mission_segment_format{}.read( ptr );
        }
        else if( typeid(format) == typeid(stanag_4607_dwell_segment_format) )
        {
          message = stanag_4607_dwell_segment_format{}.read( ptr );
        }

        //std::visit([](const auto &x)
        //{ std::cout << x << std::endl; }, message);

        segments.insert(it, message);
        it = segments.end();

        bytes_read_in_packet += segment_size;
    }

    stanag_4607_packet packet;
    packet.header = packet_header_data;
    packet.segment_headers = segment_headers;
    packet.segments = segments;

    return packet;
}

// ----------------------------------------------------------------------------
std::vector< stanag_4607_packet >
read_stanag_4607_data( std::vector< uint8_t > input_bytes )
{
  auto ptr = &*input_bytes.cbegin();

  std::vector< stanag_4607_packet > result;
  auto it = result.begin();

  while( ptr != &*input_bytes.cend() )
  {
    stanag_4607_packet_format packet;
    stanag_4607_packet packet_data = packet.read( ptr );
    result.insert( it, packet_data );
    it = result.end();
  }

  return result;
}

// ----------------------------------------------------------------------------
std::vector< stanag_4607_packet >
read_stanag_4607_file( std::string fn )
{
    std::ifstream rf (fn, std::ios::binary);
    if( !rf ) std::cerr << "DID NOT OPEN FILE " << fn << std::endl;

    std::vector<uint8_t> input_bytes((std::istreambuf_iterator<char>(rf)),
                                      std::istreambuf_iterator<char>());

    auto result = read_stanag_4607_data( input_bytes );
    return result;
}


} // Namespace stanag

} // Namespace arrows

} // Namespace kwiver
