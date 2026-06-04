// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief GDAL image_io implementation

#include "image_io.h"

#include <arrows/gdal/image_container.h>

#include <vital/exceptions/algorithm.h>
#include <vital/logger/logger.h>
#include <vital/types/geodesy.h>
#include <vital/vital_config.h>

#include <gdal_priv.h>

#include <filesystem>
#include <iomanip>
#include <sstream>

namespace kwiver {

namespace arrows {

namespace gdal {

namespace {

// ----------------------------------------------------------------------------
template < class T >
std::unique_ptr< vital::image_pixel_traits >
traits_of()
{
  return std::make_unique< vital::image_pixel_traits_of< T > >();
}

// ----------------------------------------------------------------------------
std::vector< std::string >
get_nitf_tres( vital::image_container_sptr const& data )
{
  std::vector< std::string > tres;
  auto const metadata = data->get_metadata();
  if( !metadata )
  {
    return tres;
  }

  std::vector< std::string > blocka_locs;
  for( auto const tag : {
            vital::VITAL_META_NITF_BLOCKA_FRLC_LOC_01,
            vital::VITAL_META_NITF_BLOCKA_LRLC_LOC_01,
            vital::VITAL_META_NITF_BLOCKA_LRFC_LOC_01,
            vital::VITAL_META_NITF_BLOCKA_FRFC_LOC_01 } )
  {
    if( auto const entry = metadata->find( tag ) )
    {
      auto const& value = entry.get< std::string >();
      if( value.size() != 21 )
      {
        blocka_locs.clear();
        break;
      }
      blocka_locs.emplace_back( value );
    }
  }

  if( auto const& corners_entry =
        metadata->find( vital::VITAL_META_CORNER_POINTS );
      blocka_locs.size() != 4 && corners_entry )
  {
    blocka_locs.clear();

    auto const points =
      corners_entry
      .get< vital::geo_polygon >()
      .polygon( vital::SRID::lat_lon_WGS84 )
      .get_vertices();
    for( size_t i : { 1, 2, 3, 0 } )
    {
      auto const& point = points[ i ];
      auto const lon = point[ 0 ];
      auto const lat = point[ 1 ];
      std::string str;
      if( std::isfinite( lat ) && std::isfinite( lon ) )
      {
        std::stringstream ss;
        ss << std::fixed << std::setprecision( 6 ) << std::setfill( '0' );
        ss << ( lat < 0 ? '-' : '+' ) << std::setw( 9 ) << std::abs( lat );
        ss << ( lon < 0 ? '-' : '+' ) << std::setw( 10 ) << std::abs( lon );
        str = ss.str();
      }
      else
      {
        str = std::string( 21, ' ' );
      }

      if( str.size() != 21 )
      {
        blocka_locs.clear();
        break;
      }
      blocka_locs.emplace_back( std::move( str ) );
    }
  }

  if( blocka_locs.size() == 4 )
  {
    std::stringstream ss;
    ss
      << "TRE=BLOCKA=0100000"
      << std::setfill( '0' ) << std::setw( 5 ) << data->height()
      << "                      "
      << blocka_locs[ 0 ] << blocka_locs[ 1 ]
      << blocka_locs[ 2 ] << blocka_locs[ 3 ]
      << "010.0";
    tres.emplace_back( ss.str() );
  }

  return tres;
}

} // namespace <anonymous>

/// Load image image from the file
///
/// \param filename the path to the file the load
/// \returns an image container refering to the loaded image
vital::image_container_sptr
image_io
::load_( const std::string& filename ) const
{
  return vital::image_container_sptr( new gdal::image_container( filename ) );
}

/// Save image image to a file
///
/// \param filename the path to the file to save.
/// \param data The image container refering to the image to write.
void
image_io
::save_( const std::string& filename, vital::image_container_sptr data ) const
{
  if( !data )
  {
    throw std::runtime_error( "GDAL image_io.save() given null image" );
  }

  auto const image = data->get_image();
  auto const metadata = data->get_metadata();

  GDALAllRegister();

  GDALDriverH driver = nullptr;
  std::filesystem::path filepath( filename );
  auto extension = filepath.extension().string();
  std::transform(
    extension.begin(), extension.end(), extension.begin(),
    []( unsigned char c ){ return std::tolower( c ); } );

  std::string driver_name;
  static std::map< std::string, std::string > const extension_map = {
    { ".nitf", "NITF" },
    { ".ntf", "NITF" },
    { ".tif", "GTiff" },
    { ".tiff", "GTiff" }, };

  if( auto const it = extension_map.find( extension );
      it != extension_map.end() )
  {
    driver_name = it->second;
  }

  if( !driver_name.empty() )
  {
    driver = GDALGetDriverByName( driver_name.c_str() );
  }

  if( !driver )
  {
    throw std::runtime_error(
      "Failed to load GDAL driver for extension: " + extension );
  }

  GDALDataType data_type = GDT_Unknown;
  auto const& pixel_traits = image.pixel_traits();
  for( auto const& [ vital_type, gdal_type ] : {
          std::make_pair( traits_of< uint8_t >(), GDT_Byte ),
          std::make_pair( traits_of< uint16_t >(), GDT_UInt16 ),
          std::make_pair( traits_of< int16_t >(), GDT_Int16 ),
          std::make_pair( traits_of< uint32_t >(), GDT_UInt32 ),
          std::make_pair( traits_of< int32_t >(), GDT_Int32 ),
          std::make_pair( traits_of< float >(), GDT_Float32 ),
          std::make_pair( traits_of< double >(), GDT_Float64 ), } )
  {
    if( pixel_traits == *vital_type )
    {
      data_type = gdal_type;
      break;
    }
  }

  if( data_type == GDT_Unknown )
  {
    throw std::runtime_error( "Pixel traits not convertible to GDAL" );
  }

  std::vector< std::string > create_options;
  if( driver_name == "NITF" )
  {
    create_options = get_nitf_tres( data );
    create_options.emplace_back( "ICORDS=G" );
  }

  std::vector< char const* > create_option_ptrs;
  create_option_ptrs.reserve( create_options.size() );
  for( auto const& create_option : create_options )
  {
    create_option_ptrs.emplace_back( create_option.c_str() );
  }

  if( create_option_ptrs.size() )
  {
    create_option_ptrs.emplace_back( nullptr );
  }

  auto const dataset =
    GDALCreate(
      driver, filename.c_str(),
      data->width(), data->height(), data->depth(), data_type,
      create_option_ptrs.empty() ? nullptr : create_option_ptrs.data() );

  if( !dataset )
  {
    throw std::runtime_error( "Failed to create GDAL dataset from image" );
  }

  auto const err =
    GDALDatasetRasterIO(
      dataset, GF_Write, 0, 0, data->width(), data->height(),
      const_cast< void* >( image.first_pixel() ), data->width(), data->height(),
      data_type, data->depth(), nullptr,
      image.w_step(), image.h_step(), image.d_step() );

  if( c_nodata_enabled )
  {
    for( size_t i = 0; i < data->depth(); ++i )
    {
      GDALSetRasterNoDataValue(
        GDALGetRasterBand( dataset, i + 1 ), c_nodata_value );
    }
  }

  vital::metadata_item corners_entry{ vital::VITAL_META_UNKNOWN, 0 };
  if( metadata &&
      ( corners_entry = metadata->find( vital::VITAL_META_CORNER_POINTS ) ) )
  {
    auto const points =
      corners_entry
      .get< vital::geo_polygon >()
      .polygon( vital::SRID::lat_lon_WGS84 )
      .get_vertices();

    // Convert corner points into GDAL ground control points
    auto const w = static_cast< double >( data->width() );
    auto const h = static_cast< double >( data->height() );
    GDAL_GCP gcps[ 4 ];
    GDALInitGCPs( 4, gcps );
    for( size_t i = 0; i < 4; ++i )
    {
      gcps[ i ].dfGCPX = points[ i ][ 0 ];
      gcps[ i ].dfGCPY = points[ i ][ 1 ];
      gcps[ i ].dfGCPPixel = ( i == 1 || i == 2 ) ? w : 0.0;
      gcps[ i ].dfGCPLine = ( i == 2 || i == 3 ) ? h : 0.0;
    }

    // Use ground control points to map image into geographical space
    double geo_transform[ 6 ];
    if( GDALGCPsToGeoTransform( 4, gcps, geo_transform, true ) )
    {
      GDALSetGeoTransform( dataset, geo_transform );
    }
    else
    {
      LOG_DEBUG(
        logger(),
        "Failed to convert corner points into geo-transform" );
    }
    GDALDeinitGCPs( 4, gcps );

    // Set coordinate system
    OGRSpatialReference projection;
    char* projection_wkt = nullptr;
    if( projection.SetWellKnownGeogCS( "WGS84" ) == OGRERR_NONE &&
        projection.exportToWkt( &projection_wkt ) == OGRERR_NONE )
    {
      GDALSetProjection( dataset, projection_wkt );
      CPLFree( projection_wkt );
    }
  }

  GDALClose( dataset );

  if( err != CE_None )
  {
    throw std::runtime_error( "Failed to copy image data" );
  }
}

} // end namespace gdal

} // end namespace arrows

} // end namespace kwiver
