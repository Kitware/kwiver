// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV blob class.

#include "klv_blob.h"

#include <vital/exceptions.h>

#include <iomanip>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_blob
::klv_blob()
{}

// ----------------------------------------------------------------------------
klv_blob
::klv_blob( std::initializer_list< uint8_t > const& bytes ) : bytes{ bytes }
{}

// ----------------------------------------------------------------------------
klv_blob
::klv_blob( klv_bytes_t const& bytes ) : bytes{ bytes }
{}

// ----------------------------------------------------------------------------
klv_bytes_t&
klv_blob
::operator*()
{
  return bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t const&
klv_blob
::operator*() const
{
  return bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t*
klv_blob
::operator->()
{
  return &bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t const*
klv_blob
::operator->() const
{
  return &bytes;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_blob const& blob )
{
  auto const flags = os.flags();

  os << std::hex << std::setfill( '0' );
  auto first = true;
  for( auto const c : *blob )
  {
    if( first )
    {
      first = false;
    }
    else
    {
      os << ' ';
    }
    os << std::setw( 2 ) << static_cast< unsigned int >( c );
  }

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
bool
operator==( klv_blob const& lhs, klv_blob const& rhs )
{
  return *lhs == *rhs;
}

// ----------------------------------------------------------------------------
bool
operator<( klv_blob const& lhs, klv_blob const& rhs )
{
  return std::lexicographical_compare( lhs->cbegin(), lhs->cend(),
                                       rhs->cbegin(), rhs->cend() );
}

// ----------------------------------------------------------------------------
klv_blob
klv_read_blob( klv_read_iter_t& data, size_t length )
{
  auto const begin = data;
  auto const end = ( data += length );
  return klv_bytes_t{ begin, end };
}

// ----------------------------------------------------------------------------
void
klv_write_blob(
  klv_blob const& value, klv_write_iter_t& data, size_t max_length )
{
  if( max_length < value->size() )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing blob overruns end of data buffer" );
  }

  data = std::copy( value->cbegin(), value->cend(), data );
}

// ----------------------------------------------------------------------------
size_t
klv_blob_length( klv_blob const& value )
{
  return value->size();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
