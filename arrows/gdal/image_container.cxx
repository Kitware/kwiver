// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief GDAL image_container implementation

#include "image_container.h"

#include <vital/exceptions/io.h>
#include <vital/types/geodesy.h>
#include <vital/types/metadata_traits.h>

#include <charconv>
#include <optional>
#include <regex>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace gdal {

namespace {

// ----------------------------------------------------------------------------
void
add_rpc_metadata( char* raw_md, vital::metadata_sptr md )
{
  std::istringstream md_string( raw_md );

  // Get the key
  std::string key;
  if( !std::getline( md_string, key, '=' ) )
  {
    return;
  }

  // Get the value
  std::string value;
  if( !std::getline( md_string, value, '=' ) )
  {
    return;
  }

#define MAP_METADATA_SCALAR( GN, KN )                            \
if( key == #GN )                                                 \
{                                                                \
  md->add< vital::VITAL_META_RPC_ ## KN >( std::stod( value ) ); \
}

#define MAP_METADATA_COEFF( GN, KN )                \
if( key == #GN )                                    \
{                                                   \
  md->add< vital::VITAL_META_RPC_ ## KN >( value ); \
}

  MAP_METADATA_SCALAR( HEIGHT_OFF,   HEIGHT_OFFSET )
  MAP_METADATA_SCALAR( HEIGHT_SCALE, HEIGHT_SCALE )
  MAP_METADATA_SCALAR( LONG_OFF,     LONG_OFFSET )
  MAP_METADATA_SCALAR( LONG_SCALE,   LONG_SCALE )
  MAP_METADATA_SCALAR( LAT_OFF,      LAT_OFFSET )
  MAP_METADATA_SCALAR( LAT_SCALE,    LAT_SCALE )
  MAP_METADATA_SCALAR( LINE_OFF,     ROW_OFFSET )
  MAP_METADATA_SCALAR( LINE_SCALE,   ROW_SCALE )
  MAP_METADATA_SCALAR( SAMP_OFF,     COL_OFFSET )
  MAP_METADATA_SCALAR( SAMP_SCALE,   COL_SCALE )

  MAP_METADATA_COEFF( LINE_NUM_COEFF, ROW_NUM_COEFF )
  MAP_METADATA_COEFF( LINE_DEN_COEFF, ROW_DEN_COEFF )
  MAP_METADATA_COEFF( SAMP_NUM_COEFF, COL_NUM_COEFF )
  MAP_METADATA_COEFF( SAMP_DEN_COEFF, COL_DEN_COEFF )

#undef MAP_METADATA_SCALAR
#undef MAP_METADATA_COEFF
}

void
add_nitf_metadata( char* raw_md, vital::metadata_sptr md )
{
  std::istringstream md_string( raw_md );

  // Get the key
  std::string key;
  if( !std::getline( md_string, key, '=' ) )
  {
    return;
  }

  // Get the value
  std::string value;
  if( !std::getline( md_string, value, '=' ) )
  {
    return;
  }

#define MAP_METADATA_COEFF( GN, KN )                 \
if( key == #GN )                                     \
{                                                    \
  md->add< vital::VITAL_META_NITF_ ## KN >( value ); \
}

  MAP_METADATA_COEFF( NITF_IDATIM, IDATIM )
  MAP_METADATA_COEFF( NITF_BLOCKA_FRFC_LOC_01, BLOCKA_FRFC_LOC_01 )
  MAP_METADATA_COEFF( NITF_BLOCKA_FRLC_LOC_01, BLOCKA_FRLC_LOC_01 )
  MAP_METADATA_COEFF( NITF_BLOCKA_LRLC_LOC_01, BLOCKA_LRLC_LOC_01 )
  MAP_METADATA_COEFF( NITF_BLOCKA_LRFC_LOC_01, BLOCKA_LRFC_LOC_01 )
  MAP_METADATA_COEFF( NITF_IMAGE_COMMENTS, IMAGE_COMMENTS )

#undef MAP_METADATA_SCALAR
#undef MAP_METADATA_COEFF
}

vital::polygon::point_t
apply_geo_transform( double gt[], double x, double y )
{
  vital::polygon::point_t retVal;
  retVal[ 0 ] = gt[ 0 ] + gt[ 1 ] * x + gt[ 2 ] * y;
  retVal[ 1 ] = gt[ 3 ] + gt[ 4 ] * x + gt[ 5 ] * y;
  return retVal;
}

std::optional< vital::vector_2d >
blocka_to_point( std::string const& s )
{
  // Required size
  if( s.size() != 21 )
  {
    return std::nullopt;
  }

  std::smatch match;

  // Check for decimal coordinates
  static std::regex decimal_pattern(
    "^([+-])([0-9]{2}\\.[0-9]*)-*([+-])([0-9]{3}\\.[0-9]*)-*$" );
  if( std::regex_match( s, match, decimal_pattern ) )
  {
    // std::from_chars doesn't parse leading '+', so we have to deal with the
    // sign manually
    auto const lat_sign_str = match[ 1 ].str();
    auto const lat_str = match[ 2 ].str();
    auto const lon_sign_str = match[ 3 ].str();
    auto const lon_str = match[ 4 ].str();

    // Parse values directly as doubles
    double lat, lon;
    if(
      std::from_chars(
        &*lat_str.begin(), &*lat_str.end(), lat ).ec != std::errc{} ||
      std::from_chars(
        &*lon_str.begin(), &*lon_str.end(), lon ).ec != std::errc{} )
    {
      return std::nullopt;
    }

    return vital::vector_2d{
      lon_sign_str == "-" ? -lon : lon,
      lat_sign_str == "-" ? -lat : lat };
  }

  // Check for degrees-minutes-seconds coordinates
  static std::regex sexagesimal_pattern(
    "^([NS])([0-9-]{6})\\.([0-9-]{2})([EW])([0-9-]{7})\\.([0-9-]{2})$" );
  if( std::regex_match( s, match, sexagesimal_pattern ) )
  {
    auto const parse_sexagesimal =
      []( bool sign, std::string const& dms_str, std::string const& csec_str )
      -> std::optional< double > {
        // Degrees can be 2 or 3 digits
        constexpr auto min_digits = 2;
        constexpr auto sec_digits = 2;
        auto const deg_digits = dms_str.size() - min_digits - sec_digits;

        // Unknown portions (lower precision) can be blanked out by '-'s
        auto pos = dms_str.find_first_of( '-' );
        if( pos != std::string::npos && pos < deg_digits )
        {
          // Blanked out degrees = useless
          return std::nullopt;
        }
        auto ptr = dms_str.data();

        // Parse degrees
        size_t deg = 0, min = 0, sec = 0, csec = 0;
        if( std::from_chars( ptr, ptr + deg_digits, deg ).ec != std::errc{} )
        {
          // Illegible degrees = useless
          return std::nullopt;
        }
        ptr += deg_digits;

        auto const calculate_result =
          [ & ]() -> double {
            auto const result_unsigned =
              deg + min / 60.0 + sec / 3600.0 + csec / 360000.0;
            return sign ? -result_unsigned : result_unsigned;
          };

        // Parse minutes
        if( ( pos != std::string::npos && pos < deg_digits + min_digits ) ||
            std::from_chars( ptr, ptr + min_digits, min ).ec != std::errc{} )
        {
          return calculate_result();
        }
        ptr += min_digits;

        // Parse seconds
        if( ( pos != std::string::npos && pos < dms_str.size() ) ||
            std::from_chars( ptr, ptr + sec_digits, sec ).ec != std::errc{} )
        {
          return calculate_result();
        }

        // Parse centiseconds
        pos = csec_str.find_first_of( '-' );
        if( pos == 0 )
        {
          // Centiseconds blanked out
          return calculate_result();
        }
        ptr = csec_str.data();

        if( pos == 1 )
        {
          // Only second digit blanked out
          if( std::from_chars( ptr, ptr + 1, csec ).ec == std::errc{} )
          {
            csec *= 10;
          }
          return calculate_result();
        }

        // On failure, centiseconds will be left as 0 as desired
        std::from_chars( ptr, ptr + 2, csec );

        return calculate_result();
      };

    auto const lat_ns = match[ 1 ].str();
    auto const lat_dms = match[ 2 ].str();
    auto const lat_csec = match[ 3 ].str();

    auto const lon_ew = match[ 4 ].str();
    auto const lon_dms = match[ 5 ].str();
    auto const lon_csec = match[ 6 ].str();

    auto const lat = parse_sexagesimal( lat_ns == "S", lat_dms, lat_csec );
    auto const lon = parse_sexagesimal( lon_ew == "W", lon_dms, lon_csec );

    if( lat && lon )
    {
      return vital::vector_2d{ *lon, *lat };
    }

    return std::nullopt;
  }

  return std::nullopt;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
image_container
::image_container( const std::string& filename )
{
  GDALAllRegister();

  gdal_dataset_.reset(
    static_cast< GDALDataset* >( GDALOpen( filename.c_str(), GA_ReadOnly ) ) );

  if( !gdal_dataset_ )
  {
    VITAL_THROW( vital::invalid_file, filename, "GDAL could not load file." );
  }

  // Get image pixel traits based on the GDAL raster type.
  // TODO: deal or provide warning if bands have different types.
  auto bandType = gdal_dataset_->GetRasterBand( 1 )->GetRasterDataType();
  switch( bandType )
  {
    case ( GDT_Byte ):
    {
      pixel_traits_ = vital::image_pixel_traits_of< uint8_t >();
      break;
    }
    case ( GDT_UInt16 ):
    {
      pixel_traits_ = vital::image_pixel_traits_of< uint16_t >();
      break;
    }
    case ( GDT_Int16 ):
    {
      pixel_traits_ = vital::image_pixel_traits_of< int16_t >();
      break;
    }
    case ( GDT_UInt32 ):
    {
      pixel_traits_ = vital::image_pixel_traits_of< uint32_t >();
      break;
    }
    case ( GDT_Int32 ):
    {
      pixel_traits_ = vital::image_pixel_traits_of< int32_t >();
      break;
    }
    case ( GDT_Float32 ):
    {
      pixel_traits_ = vital::image_pixel_traits_of< float >();
      break;
    }
    case ( GDT_Float64 ):
    {
      pixel_traits_ = vital::image_pixel_traits_of< double >();
      break;
    }
    default:
    {
      std::stringstream ss;
      ss << "kwiver::arrows::gdal::image_io::load(): "
         << "Unknown or unsupported pixal type: "
         << GDALGetDataTypeName( bandType );
      VITAL_THROW( vital::image_type_mismatch_exception, ss.str() );
      break;
    }
  }

  vital::metadata_sptr md = std::make_shared< vital::metadata >();

  md->add< kwiver::vital::VITAL_META_IMAGE_URI >( filename );

  // Get RPC metadata
  char** rpc_metadata = gdal_dataset_->GetMetadata( "RPC" );
  if( CSLCount( rpc_metadata ) > 0 )
  {
    for( int i = 0; rpc_metadata[ i ] != nullptr; ++i )
    {
      add_rpc_metadata( rpc_metadata[ i ], md );
    }
  }

  // Get NITF metadata
  char** nitf_metadata = gdal_dataset_->GetMetadata( "" );
  if( CSLCount( nitf_metadata ) > 0 )
  {
    for( int i = 0; nitf_metadata[ i ] != nullptr; ++i )
    {
      add_nitf_metadata( nitf_metadata[ i ], md );
    }
  }

  if( md->has( vital::VITAL_META_NITF_BLOCKA_FRFC_LOC_01 ) )
  {
    // Parse corner points directly
    std::vector< vital::vector_2d > points;
    for( auto const tag : {
            vital::VITAL_META_NITF_BLOCKA_FRFC_LOC_01,
            vital::VITAL_META_NITF_BLOCKA_FRLC_LOC_01,
            vital::VITAL_META_NITF_BLOCKA_LRLC_LOC_01,
            vital::VITAL_META_NITF_BLOCKA_LRFC_LOC_01, } )
    {
      auto const& entry = md->find( tag );
      if( !entry )
      {
        break;
      }

      auto const& value = entry.get< std::string >();
      if( auto const point = blocka_to_point( value ) )
      {
        points.emplace_back( *point );
      }
    }

    if( points.size() == 4 )
    {
      md->add< vital::VITAL_META_CORNER_POINTS >(
        vital::geo_polygon{ points, vital::SRID::lat_lon_WGS84 } );
    }
  }

  if( !md->has( vital::VITAL_META_CORNER_POINTS ) )
  {
    // Get geotransform and calculate corner points
    double geo_transform[ 6 ];
    gdal_dataset_->GetGeoTransform( geo_transform );

    OGRSpatialReference osrs;
    osrs.importFromWkt( gdal_dataset_->GetProjectionRef() );

    // If coordinate system available - calculate corner points.
    if( osrs.GetAuthorityCode( "GEOGCS" ) )
    {
      vital::polygon points;
      const double h = static_cast< double >( this->height() );
      const double w = static_cast< double >( this->width() );
      points.push_back( apply_geo_transform( geo_transform, 0, 0 ) );
      points.push_back( apply_geo_transform( geo_transform, w, 0 ) );
      points.push_back( apply_geo_transform( geo_transform, w, h ) );
      points.push_back( apply_geo_transform( geo_transform, 0, h ) );

      vital::geo_polygon const polygon(
        points, atoi( osrs.GetAuthorityCode( "GEOGCS" ) ) );
      md->add< vital::VITAL_META_CORNER_POINTS >( polygon );
    }
  }

  this->set_metadata( md );
}

char**
image_container
::get_raw_metadata_for_domain( const char* domain )
{
  return this->gdal_dataset_->GetMetadata( domain );
}

// ----------------------------------------------------------------------------
/// The size of the image data in bytes
size_t
image_container
::size() const
{
  return width() * height() * depth() * pixel_traits_.num_bytes;
}

// ----------------------------------------------------------------------------
/// Get image. Unlike other image containers must allocate memory
vital::image
image_container
::get_image() const
{
  vital::image img( width(), height(), depth(), false, pixel_traits_ );

  // Loop over bands and copy data
  CPLErr err;
  for( size_t i = 1; i <= depth(); ++i )
  {
    GDALRasterBand* band =
      gdal_dataset_->GetRasterBand( static_cast< int >( i ) );
    auto bandType = band->GetRasterDataType();
    const int h = static_cast< int >( this->height() );
    const int w = static_cast< int >( this->width() );
    err = band->RasterIO(
      GF_Read, 0, 0, w, h,
      static_cast< void* >( reinterpret_cast< GByte* >(
                              img.first_pixel() ) + ( i - 1 ) * img.d_step() *
                            img.pixel_traits().num_bytes ),
      w, h, bandType, 0, 0 );
    // TODO Error checking on return value
    // this line silences unused variable warnings
    ( void ) err;
  }

  return img;
}

// ----------------------------------------------------------------------------
/// Get cropped view of image. Unlike other image containers must allocate
/// memory
vital::image
image_container
::get_image(
  size_t x_offset, size_t y_offset,
  size_t width, size_t height ) const
{
  vital::image img( width, height, depth(), false, pixel_traits_ );

  // Loop over bands and copy data
  CPLErr err;
  for( size_t i = 1; i <= depth(); ++i )
  {
    GDALRasterBand* band = gdal_dataset_->GetRasterBand( i );
    auto bandType = band->GetRasterDataType();
    err = band->RasterIO(
      GF_Read, x_offset, y_offset, width, height,
      static_cast< void* >( reinterpret_cast< GByte* >(
                              img.first_pixel() ) + ( i - 1 ) * img.d_step() *
                            img.pixel_traits().num_bytes ),
      width, height, bandType, 0, 0 );

    ( void ) err;
  }

  return img;
}

} // end namespace gdal

} // end namespace arrows

} // end namespace kwiver
