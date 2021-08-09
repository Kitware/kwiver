// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV blob class's templated functions.

#include "klv_blob.h"

#include <vital/exceptions/metadata.h>

#include <algorithm>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
#define KLV_ASSERT_UINT8_ITERATOR( ITER )                         \
  static_assert(                                                  \
    std::is_same< typename std::decay< decltype( *ITER ) >::type, \
                  uint8_t >::value,                               \
    "iterator must point to uint8_t" )

// ----------------------------------------------------------------------------
template < class Iterator >
klv_blob
klv_read_blob( Iterator& data, size_t length )
{
  KLV_ASSERT_UINT8_ITERATOR( data );
  auto const begin = data;
  auto const end = ( data += length );
  return klv_bytes_t{ begin, end };
}

// ----------------------------------------------------------------------------
template < class Iterator >
void
klv_write_blob( klv_blob const& value, Iterator& data, size_t max_length )
{
  KLV_ASSERT_UINT8_ITERATOR( data );

  if( max_length < value->size() )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing blob overruns end of data buffer" );
  }

  data = std::copy( value->cbegin(), value->cend(), data );
}

#undef KLV_ASSERT_UINT8_ITERATOR

} // namespace klv

} // namespace arrows

} // namespace kwiver
