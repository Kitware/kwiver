// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of save wrapping functionality

#include "pointcloud_io.h"

#include <kwiversys/SystemTools.hxx>
#include <vital/algo/algorithm.txx>
#include <vital/exceptions/io.h>

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::pointcloud_io );
/// \endcond

namespace kwiver {
namespace vital {
namespace algo {
pointcloud_io
  ::pointcloud_io()
{
  attach_logger( "algo.pointcloud_io" );
}

pointcloud_d
pointcloud_io
  ::load( std::string const& filename ) const
{
  // Make sure that the given file path exists and is a file.
  if( !kwiversys::SystemTools::FileExists( filename ) )
  {
    VITAL_THROW( path_not_exists, filename );
  }
  else if( kwiversys::SystemTools::FileIsDirectory( filename ) )
  {
    VITAL_THROW( path_not_a_file, filename );
  }

  return this->load_( filename );
}

void
pointcloud_io
  ::set_local_geo_cs( vital::local_geo_cs const& lgcs )
{
  LOG_WARN( logger(), "Setting local geo cs is not implemented." );
}

void
pointcloud_io
  ::save( vital::path_t const& filename,
          std::vector< vital::vector_3d > const& points,
          std::vector< vital::rgb_color > const& colors )
{
  // Make sure that the given file path's containing directory exists and is
  // actually a directory.
  std::string containing_dir = kwiversys::SystemTools::GetFilenamePath(
    kwiversys::SystemTools::CollapseFullPath( filename ) );

  if( !kwiversys::SystemTools::FileExists( containing_dir ) )
  {
    VITAL_THROW( path_not_exists, containing_dir );
  }
  else if( !kwiversys::SystemTools::FileIsDirectory( containing_dir ) )
  {
    VITAL_THROW( path_not_a_directory, containing_dir );
  }

  // Make sure that the given path is a file.
  if( kwiversys::SystemTools::FileIsDirectory( filename ) )
  {
    VITAL_THROW( path_not_a_file, filename );
  }

  this->save_( filename, points, colors );
}

void
pointcloud_io
  ::save( std::string const& filename,
          vital::landmark_map_sptr const& landmarks )
{
  std::vector< vital::vector_3d > points;
  std::vector< vital::rgb_color > colors;
  points.reserve( landmarks->size() );
  colors.reserve( landmarks->size() );
  for( auto lm : landmarks->landmarks() )
  {
    points.push_back( lm.second->loc() );
    colors.push_back( lm.second->color() );
  }
  this->save( filename, points, colors );
}
} // namespace algo
} // namespace vital
} // namespace kwiver
