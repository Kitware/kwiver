// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of load/save wrapping functionality.

#include "image_io.h"

#include <vital/algo/algorithm.txx>
#include <vital/exceptions/io.h>
#include <vital/vital_config.h>
#include <vital/vital_types.h>

#include <kwiversys/SystemTools.hxx>

namespace kwiver::vital::algo {

const algorithm_capabilities::capability_name_t image_io::HAS_TIME(
  "has-time" );

image_io
::image_io()
{
  attach_logger( "algo.image_io" );
}

image_container_sptr
image_io
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
image_io
::save( std::string const& filename, image_container_sptr data ) const
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

  this->save_( filename, data );
}

metadata_sptr
image_io
::load_metadata( std::string const& filename ) const
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

  return this->load_metadata_( filename );
}

const vital::algorithm_capabilities&
image_io
::get_implementation_capabilities() const
{
  return this->m_capabilities;
}

void
image_io
::set_capability( algorithm_capabilities::capability_name_t const& name,
                  bool val )
{
  this->m_capabilities.set_capability( name, val );
}

metadata_sptr
image_io
::load_metadata_( [[maybe_unused]] std::string const& filename ) const
{
  // No metadata-only loading by default.
  return nullptr;
}

} // namespace
