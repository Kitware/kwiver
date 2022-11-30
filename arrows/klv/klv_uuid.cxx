// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV UUID parser.

#include "klv_uuid.h"

#include <algorithm>
#include <iomanip>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_uuid
::klv_uuid() : bytes{ 0 }
{}

// ----------------------------------------------------------------------------
klv_uuid
::klv_uuid( std::initializer_list< uint8_t > const& bytes ) : bytes{}
{
  std::copy( bytes.begin(), bytes.end(), this->bytes.begin() );
}

// ----------------------------------------------------------------------------
klv_uuid
::klv_uuid( std::array< uint8_t, 16 > const& bytes ) : bytes( bytes )
{}

// ----------------------------------------------------------------------------
// Prints like 0123-4567-89AB-CDEF-0123-4567-89AB-CDEF
std::ostream&
operator<<( std::ostream& os, klv_uuid const& value )
{
  auto const flags = os.flags();

  for( size_t i = 0; i < value.bytes.size(); ++i )
  {
    if( i != 0 && i % 2 == 0 )
    {
      os << '-';
    }
    os << std::hex << std::setfill( '0' ) << std::setw( 2 );
    os << static_cast< unsigned int >( value.bytes[ i ] );
  }

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_uuid,
  &klv_uuid::bytes
  )

// ----------------------------------------------------------------------------
klv_uuid
klv_read_uuid( klv_read_iter_t& data, size_t max_length )
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
void
klv_write_uuid( klv_uuid const& value, klv_write_iter_t& data,
                size_t max_length )
{
  if( max_length < 16 )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing UUID overflows buffer" );
  }
  data = std::copy( value.bytes.begin(), value.bytes.end(), data );
}

// ----------------------------------------------------------------------------
size_t
klv_uuid_length()
{
  return 16;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
