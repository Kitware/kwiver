// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of camera rig I/O functions

#include "camera_rig_io.h"
#include "camera_io.h"

#include <vital/exceptions.h>

#include <kwiversys/SystemTools.hxx>

namespace kwiver {
namespace vital {

camera_rig_sptr
read_camera_rig( path_list_t const & cam_files )
{
  camera_rig_sptr rig ( new camera_rig() );
  for ( auto const & cf : cam_files )
  {
    try
    {
      rig->add( cf, read_krtd_file( cf ) );
    }
    catch ( const file_not_found_exception& )
    {
      std::clog << "warning: unable to find " << cf << std::endl;
      continue;
    }
  }
  if ( rig->empty() )
  {
    VITAL_THROW( invalid_data, "no cameras initialized from the given list of krtd files" ) ;
  }
  return rig;
}

} // vital
} // kwiver
