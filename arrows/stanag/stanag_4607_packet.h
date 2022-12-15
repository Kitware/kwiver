// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Defines a STANAG 4607 packet and packet header

#ifndef KWIVER_ARROWS_STANAG_4607_PACKET_H_
#define KWIVER_ARROWS_STANAG_4607_PACKET_H_

#include <arrows/stanag/kwiver_algo_stanag_export.h>

#include "stanag_4607_segments.h"
#include "stanag_util.h"

namespace ka = kwiver::arrows;

#include <map>
#include <memory>
#include <ostream>
#include <variant>
#include <vector>

namespace kwiver {

namespace arrows {

namespace stanag {

// ----------------------------------------------------------------------------
/// Indicates the classification level of a packet
enum KWIVER_ALGO_STANAG_EXPORT stanag_4607_security_classification
{
  STANAG_4607_SECURITY_CLASS_TOP_SECRET      = 1,
  STANAG_4607_SECURITY_CLASS_SECRET          = 2,
  STANAG_4607_SECURITY_CLASS_CONFIDENTIAL    = 3,
  STANAG_4607_SECURITY_CLASS_RESTRICTED      = 4,
  STANAG_4607_SECURITY_CLASS_UNCLASSIFIED    = 5,
  STANAG_4607_SECURITY_CLASS_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_security_classification const& value );

// ----------------------------------------------------------------------------
/// Indicates additional control and/or handling instructions associated
/// with the GMTI data
enum KWIVER_ALGO_STANAG_EXPORT stanag_4607_security_code
{
  STANAG_4607_SECURITY_CODE_NONE                 = 0x0000,
  STANAG_4607_SECURITY_CODE_NOCONTRACT           = 0x0001,
  STANAG_4607_SECURITY_CODE_ORCON                = 0x0002,
  STANAG_4607_SECURITY_CODE_PROPIN               = 0x0004,
  STANAG_4607_SECURITY_CODE_WNINTEL              = 0x0008,
  STANAG_4607_SECURITY_CODE_NATIONAL_ONLY        = 0x0010,
  STANAG_4607_SECURITY_CODE_LIMDIS               = 0x0020,
  STANAG_4607_SECURITY_CODE_FOUO                 = 0x0040,
  STANAG_4607_SECURITY_CODE_EFTO                 = 0x0080,
  STANAG_4607_SECURITY_CODE_LIM_OFF_USE          = 0x0100,
  STANAG_4607_SECURITY_CODE_NONCOMPARTMENT       = 0x0200,
  STANAG_4607_SECURITY_CODE_SPECIAL_CONTROL      = 0x0400,
  STANAG_4607_SECURITY_CODE_SPECIAL_INTEL        = 0x0800,
  STANAG_4607_SECURITY_CODE_WARNING_NOTICE       = 0x1000,
  STANAG_4607_SECURITY_CODE_REL_NATO             = 0x2000,
  STANAG_4607_SECURITY_CODE_REL_4_EYES           = 0x4000,
  STANAG_4607_SECURITY_CODE_REL_9_EYES           = 0x8000,
  STANAG_4607_SECURITY_CODE_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_security_code const& value );

// ----------------------------------------------------------------------------
/// Security information for the packet
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_packet_security
{
  stanag_4607_security_classification classification;
  std::string class_system;
  stanag_4607_security_code code;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_packet_security const& value );


// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_packet_security )

// ----------------------------------------------------------------------------
/// Indicating whether the data contained in this packet is from a real-world
/// military operation or from an exercise, and whether the data is real,
/// simulated, or synthesized.
enum KWIVER_ALGO_STANAG_EXPORT stanag_4607_exercise_indicator
{
  STANAG_4607_EXERCISE_IND_OPERATION_REAL           = 0,
  STANAG_4607_EXERCISE_IND_OPERATION_SIMULATED      = 1,
  STANAG_4607_EXERCISE_IND_OPERATION_SYNTHESIZED    = 2,
  // Note: 2-127 are reserved
  STANAG_4607_EXERCISE_IND_EXERCISE_REAL            = 128,
  STANAG_4607_EXERCISE_IND_EXERCISE_SIMULATED       = 129,
  STANAG_4607_EXERCISE_IND_EXERCISE_SYNTHESIZED     = 130,
  // Note: 131-255 are reserved
  STANAG_4607_EXERCISE_IND_ENUM_END                 = 256,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_exercise_indicator const& value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_packet_security const& value );

// ----------------------------------------------------------------------------
/// Provides basic information concerning the platform, the job, the mission,
/// nationality, security, and the length of the packet.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_packet_header
{
  std::string version_id;
  size_t packet_size; // Includes header size
  std::string nationality;
  stanag_4607_packet_security packet_security;
  stanag_4607_exercise_indicator exercise_indicator;
  std::string platform_id;
  int mission_id;
  int job_id;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_packet_header const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_packet_header )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_packet_header_format
  : public stanag_4607_packet_header
{
public:
  stanag_4607_packet_header_format();


  const size_t size = 32; // Number of  bytes in packet header

  stanag_4607_packet_header
  read( ptr_t& ptr ) const;
};


// ----------------------------------------------------------------------------
/// Top level STANAG 4607 packet
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_packet
{
  stanag_4607_packet_header header;
  std::vector< stanag_4607_segment_header > segment_headers;
  std::vector< stanag_4607_mission_segment > segments; // TODO: make this any
                                                       // segment type
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_packet const& value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, std::vector< stanag_4607_packet > const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_packet )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_packet_format
  : public stanag_4607_packet
{
public:
  stanag_4607_packet_format();

  stanag_4607_packet
  read( ptr_t& ptr ) const;
};


// ----------------------------------------------------------------------------
/// Read the input data as a list of packets
KWIVER_ALGO_STANAG_EXPORT
std::vector< stanag_4607_packet >
read_stanag_4607_data( ptr_t& ptr );

} // Namespace stanag

} // Namespace arrows

} // Namespace kwiver

#endif
