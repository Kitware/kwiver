// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV UUID struct's templated functions.

#include "klv_uuid.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class Iterator >
klv_uuid
klv_read_uuid( Iterator& data, size_t max_length )
{
  if( max_length < 16 )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "reading UUID overflows buffer" );
  }

  klv_uuid result;
  std::copy_n( data, 16, result.bytes.begin() );
  data += 16;
  return result;
}

// ----------------------------------------------------------------------------
template < class Iterator >
void
klv_write_uuid( klv_uuid const& value, Iterator& data,
                size_t max_length )
{
  if( max_length < 16 )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing UUID overflows buffer" );
  }
  data = std::copy( value.bytes.begin(), value.bytes.end(), data );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
