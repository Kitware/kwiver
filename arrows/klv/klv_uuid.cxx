// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of the KLV UUID parser.

#include "klv_uuid.h"

#include <iomanip>

namespace kwiver {

namespace arrows {

namespace klv {

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
size_t
klv_uuid_length()
{
  return 16;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
