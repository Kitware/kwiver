// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of uuid factory

#include "uuid_factory_uuid.h"

#if defined WIN32
#include <windows.h>
#else
#include <uuid/uuid.h>
#endif

namespace kwiver {

namespace arrows {

namespace uuid {

// ----------------------------------------------------------------------------
void
uuid_factory_uuid
::initialize()
{
  attach_logger( "arrows.uuid.uuid_factory_uuid" );
}

// ----------------------------------------------------------------------------
bool
uuid_factory_uuid
::check_configuration( vital::config_block_sptr VITAL_UNUSED( config ) ) const
{
  return true;
}

// ----------------------------------------------------------------------------
kwiver::vital::uid
uuid_factory_uuid
::create_uuid()
{
#if defined WIN32
  UUID new_uuid;

  UuidCreate( &new_uuid );

  const char* cc = ( const char* ) &new_uuid.Data4[ 0 ];
#else
  // This may need work to be more system independent.
  uuid_t new_uuid;

  // from libuuid
  uuid_generate( new_uuid );

  const char* cc = ( const char* ) &new_uuid[ 0 ];
#endif

  return kwiver::vital::uid( cc, sizeof( new_uuid ) );
}

} // namespace uuid

} // namespace arrows

}     // end namespace
