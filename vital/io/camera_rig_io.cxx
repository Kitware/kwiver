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

static auto logger = get_logger( "vital.camera_rig_io" );

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
      LOG_ERROR(logger, "error: unable to find " << cf);
      continue;
    }
  }
  if ( rig->empty() )
  {
    VITAL_THROW( invalid_data,
                 "no cameras initialized from the given list of files" ) ;
  }
  return rig;
}

camera_rig_stereo_sptr
read_stereo_rig( path_t const& FN )
{
  camera_rig_stereo_sptr res;
  // TODO read from FN
  return res;
}

void
write_camera_rig( camera_rig_sptr rig )
{
  if (rig==nullptr)
  {
    LOG_ERROR( logger,
     "unable to write null camera rig pointer" );
    return;
  }
  for (auto const & c: rig->cameras())
  {
    try
    {
      auto const & cam = dynamic_cast<camera_perspective const&>(*c.second);
      write_krtd_file(cam, c.first);
    }
    catch( std::exception const & e )
    {
      LOG_ERROR(logger, "unable to write " << c.first
          << ": " << e.what() );
    }
  }
}

void
write_stereo_rig( camera_rig_stereo_sptr rig,
                               std::string const & FN )
{
  // TODO write rig to FN
}

} // vital
} // kwiver
