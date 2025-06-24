// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of the image_io_tiled_multifile algorithm.

#include <arrows/core/algo/image_io_tiled_multifile.h>

#include <vital/types/tiled_image_container.h>
#include <vital/types/tiled_image_container_simple.h>

#include <filesystem>
#include <iomanip>
#include <regex>
#include <sstream>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
image_io_tiled_multifile
::~image_io_tiled_multifile()
{}

// ----------------------------------------------------------------------------
bool
image_io_tiled_multifile
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
vital::image_container_sptr
image_io_tiled_multifile
::load_( std::string const& filename ) const
{
  if( !c_image_io )
  {
    throw std::runtime_error( "No nested image I/O algorithm provided" );
  }

  std::filesystem::path path{ filename };
  path = std::filesystem::absolute( path );
  if( !path.has_filename() )
  {
    throw std::runtime_error( "No filename stub given in path" );
  }

  size_t max_y = 0;
  size_t max_x = 0;

  std::vector< std::tuple< std::filesystem::path, size_t, size_t > > paths;
  std::regex const pattern{ "^(.*)\\.([0-9]{4})\\.([0-9]{4})(.*)$" };
  for( auto const& entry :
       std::filesystem::directory_iterator{ path.parent_path() } )
  {
    auto base = entry.path();
    base.replace_extension();
    if( base.string().size() < 10 )
    {
      continue;
    }
    base = base.string().substr( 0, base.string().size() - 10 );

    auto extension = entry.path().extension();
    auto entry_filename = entry.path().string();

    std::smatch match;
    if( std::regex_match( entry_filename, match, pattern ) &&
        match[ 1 ].str() == base && match[ 4 ].str() == extension.string() )
    {
      size_t y = std::stoull( match[ 2 ].str() );
      size_t x = std::stoull( match[ 3 ].str() );
      max_y = std::max( max_y, y );
      max_x = std::max( max_x, x );
      paths.emplace_back( entry.path(), y, x );
    }
  }

  std::shared_ptr< vital::simple_tiled_image_container > result;

  for( auto const& [ tile_path, y, x ] : paths )
  {
    auto const tile = c_image_io->load( tile_path.string() );
    if( !result )
    {
      result = std::make_shared< vital::simple_tiled_image_container >(
        tile->width(), tile->height(), max_x + 1, max_y + 1, tile->depth(),
        tile->get_image().pixel_traits() );
    }
    result->set_tile( x, y, tile );
  }

  return result;
}

// ----------------------------------------------------------------------------
void
image_io_tiled_multifile
::save_(
  std::string const& filename,
  vital::image_container_sptr data ) const
{
  if( !c_image_io )
  {
    throw std::runtime_error( "No nested image I/O algorithm provided" );
  }

  auto const tiles =
    dynamic_cast< vital::tiled_image_container const* >( data.get() );
  if( !tiles )
  {
    throw std::runtime_error( "Tile multifile I/O given non-tiled image" );
  }

  std::filesystem::path path{ filename };
  path = std::filesystem::absolute( path );

  if( !path.has_filename() )
  {
    throw std::runtime_error( "No filename stub given in path" );
  }

  auto first = true;
  size_t x, y;
  while( tiles->next_tile( x, y, first ) )
  {
    first = false;

    std::filesystem::path out_path{ path };
    std::stringstream ss;
    ss
      << std::setfill( '0' )
      << std::setw( 4 ) << y  << "."
      << std::setw( 4 ) << x << out_path.extension().string();
    out_path.replace_extension( ss.str() );
    c_image_io->save( out_path.string(), tiles->get_tile( x, y ) );
  }
}

// ----------------------------------------------------------------------------
void
image_io_tiled_multifile
::initialize()
{}

} // namespace core

} // namespace arrows

} // namespace kwiver
