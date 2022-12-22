// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Defines a STANAG 4607 segment header and the various segment types

#ifndef KWIVER_ARROWS_STANAG_4607_SEGMENTS_H_
#define KWIVER_ARROWS_STANAG_4607_SEGMENTS_H_

#include <arrows/stanag/kwiver_algo_stanag_export.h>
#include <arrows/stanag/stanag_util.h>

#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace kwiver {

namespace arrows {

namespace stanag {

namespace kv = kwiver::vital;

// ----------------------------------------------------------------------------
/// The type of message contained in the segment
enum KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_type : uint16_t
{
  STANAG_4607_SEGMENT_TYPE_MISSION                  = 1,
  STANAG_4607_SEGMENT_TYPE_DWELL                    = 2,
  STANAG_4607_SEGMENT_TYPE_HRR                      = 3,
  // Note: 4 is reserved
  STANAG_4607_SEGMENT_TYPE_JOB_DEFINITION           = 5,
  STANAG_4607_SEGMENT_TYPE_FREE_TEXT                = 6,
  STANAG_4607_SEGMENT_TYPE_LOW_REFLECTIVITY_INDEX   = 7,
  STANAG_4607_SEGMENT_TYPE_GROUP                    = 8,
  STANAG_4607_SEGMENT_TYPE_ATTACHED_TARGET          = 9,
  STANAG_4607_SEGMENT_TYPE_TEST_AND_STATUS          = 10,
  STANAG_4607_SEGMENT_TYPE_SYSTEM_SPECIFIC          = 11,
  STANAG_4607_SEGMENT_TYPE_PROCESSING_HISTORY       = 12,
  STANAG_4607_SEGMENT_TYPE_PLATFORM_LOCATION        = 13,
  // Note: 14-100 are reserved for new segments
  STANAG_4607_SEGMENT_TYPE_JOB_REQUEST              = 101,
  STANAG_4607_SEGMENT_TYPE_JOB_ACKNOWLEDGE          = 102,
  // Note: 103-127 are reserved for future use
  // Note: 128-255 are reserved for extensions
  STANAG_4607_SEGMENT_TYPE_ENUM_END                 = 256,
};

// ----------------------------------------------------------------------------
/// Identifies the type and size of the segment that follows
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_header
{
  stanag_4607_segment_type segment_type;
  size_t segment_size;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_segment_header const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_segment_header )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_header_format
  : public stanag_4607_segment_header
{
public:
  stanag_4607_segment_header_format();

  const size_t size = 5; // Number of bytes in the segment header

  stanag_4607_segment_header
  read( ptr_t& ptr ) const;
};

// ----------------------------------------------------------------------------
/// Untyped base for STANAG formats
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_data_format
{
public:
  explicit
  stanag_4607_segment_data_format() {}

  virtual
  ~stanag_4607_segment_data_format() = default;

  template < class T > T
  read( ptr_t& ptr ) const;
};

using stanag_4607_segment_data_format_sptr =
  std::shared_ptr< stanag_4607_segment_data_format >;

// ----------------------------------------------------------------------------
/// Typed base for STANAG formats
template < class T >
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_data_format_
  : public stanag_4607_segment_data_format
{
public:
  using data_type = T;

  explicit
  stanag_4607_segment_data_format_()
  {}

  virtual
  ~stanag_4607_segment_data_format_() {}

  T
  read( ptr_t& ptr ) const;
};

template < class T >
T
stanag_4607_segment_data_format
::read( ptr_t& ptr ) const
{
  return dynamic_cast< const stanag_4607_segment_data_format_< T >& >( *this )
    .read( ptr );
}

} // namespace stanag

} // namespace arrows

} // namespace kwiver

#endif
