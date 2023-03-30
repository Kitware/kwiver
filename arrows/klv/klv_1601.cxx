// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1601 parser.

#include <arrows/klv/klv_1601.h>

#include <arrows/klv/klv_1303.h>
#include <arrows/klv/klv_string.h>

#include <vital/range/iota.h>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
static auto const pixel_sdcc_internal_format =
  klv_1303_mdap_format< klv_uint_format >{};
static auto const pixel_sdcc_external_format =
  klv_1303_mdap_format< klv_lengthless_format< klv_float_format > >{ 8 };
static auto const geographic_sdcc_internal_format =
  klv_1303_mdap_format< klv_uint_format >{};
static auto const geographic_sdcc_external_format =
  klv_1303_mdap_format< klv_lengthless_format< klv_float_format > >{ 8 };

// ----------------------------------------------------------------------------
static std::vector< kv::interval< double > > const
pixel_sdcc_imap_params = {
  { 0.0, 100.0 },
  { 0.0, 100.0 },
  { -1.0, 1.0 },
  { 0.0, 100.0 },
  { 0.0, 100.0 },
  { -1.0, 1.0 } };
static std::vector< kv::interval< double > > const
geographic_sdcc_imap_params = {
  { 0.0, 650.0 },
  { 0.0, 650.0 },
  { -1.0, 1.0 },
  { 0.0, 1000.0 },
  { -1.0, 1.0 },
  { -1.0, 1.0 } };

// ----------------------------------------------------------------------------
uint64_t
imap_to_int(
  double imap_value, vital::interval< double > const& interval, size_t length )
{
  std::vector< uint8_t > bytes( length );
  auto it = &*bytes.begin();
  klv_write_imap( imap_value, interval, it, length );
  auto cit = &*bytes.cbegin();
  return klv_read_int< uint64_t >( cit, length );
}

// ----------------------------------------------------------------------------
double
int_to_imap(
  uint64_t int_value, vital::interval< double > const& interval, size_t length )
{
  std::vector< uint8_t > bytes( length );
  auto it = &*bytes.begin();
  klv_write_int( int_value, it, length );
  auto cit = &*bytes.cbegin();
  return klv_read_imap( interval, cit, length );
}

} // namespace

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1601_tag tag )
{
  return os << klv_1601_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_1601_pixel_sdcc_format
::klv_1601_pixel_sdcc_format()
{}

// ----------------------------------------------------------------------------
std::string
klv_1601_pixel_sdcc_format
::description_() const
{
  return "ST1601 Pixel SDCC MDARRAY Pack";
}

// ----------------------------------------------------------------------------
klv_1303_mdap< double >
klv_1601_pixel_sdcc_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const& format = pixel_sdcc_internal_format;
  auto const int_value = format.read_( data, length );

  klv_1303_mdap< double > result =
  {
    int_value.sizes,
    {},
    int_value.element_size,
    int_value.apa,
    int_value.apa_params_length,
    int_value.imap_params, };

  if( result.sizes.size() != 2 || result.sizes.at( 0 ) != 6 )
  {
    VITAL_THROW( kv::metadata_exception,
                 "pixel sdcc mdarray does not have correct dimensions" );
  }

  auto const count = result.sizes.at( 1 );
  for( auto const i : kvr::iota< size_t >( 6 ) )
  {
    for( auto const j : kvr::iota( count ) )
    {
      auto const int_element = int_value.elements.at( i * count + j );
      auto const& imap_params = pixel_sdcc_imap_params[ i ];
      auto const imap_element =
        int_to_imap( int_element, imap_params, result.element_size );
      result.elements.emplace_back( imap_element );
    }
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_1601_pixel_sdcc_format
::write_typed( klv_1303_mdap< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const& format = pixel_sdcc_internal_format;
  klv_1303_mdap< uint64_t > int_value =
  {
    value.sizes,
    {},
    value.element_size,
    value.apa,
    value.apa_params_length,
    value.imap_params };

  if( int_value.sizes.size() != 2 || int_value.sizes.at( 0 ) != 6 )
  {
    VITAL_THROW( kv::metadata_exception,
                 "pixel sdcc mdarray does not have correct dimensions" );
  }

  auto const count = int_value.sizes.at( 1 );
  for( auto const i : kvr::iota< size_t >( 6 ) )
  {
    for( auto const j : kvr::iota( count ) )
    {
      auto const imap_element = value.elements.at( i * count + j );
      auto const& imap_params = pixel_sdcc_imap_params[ i ];
      auto const int_element =
        imap_to_int( imap_element, imap_params, int_value.element_size );
      int_value.elements.emplace_back( int_element );
    }
  }

  format.write_( int_value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_1601_pixel_sdcc_format
::length_of_typed( klv_1303_mdap< double > const& value ) const
{
  return pixel_sdcc_external_format.length_of_( value );
}

// ----------------------------------------------------------------------------
klv_1601_geographic_sdcc_format
::klv_1601_geographic_sdcc_format()
{}

// ----------------------------------------------------------------------------
std::string
klv_1601_geographic_sdcc_format
::description_() const
{
  return "ST1601 Geographic SDCC MDARRAY Pack";
}

// ----------------------------------------------------------------------------
klv_1303_mdap< double >
klv_1601_geographic_sdcc_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const& format = geographic_sdcc_internal_format;
  auto const int_value = format.read_( data, length );

  klv_1303_mdap< double > result =
  {
    int_value.sizes,
    {},
    int_value.element_size,
    int_value.apa,
    int_value.apa_params_length,
    int_value.imap_params, };

  if( result.sizes.size() != 2 ||
      ( result.sizes.at( 0 ) != 3 && result.sizes.at( 0 ) != 6 ) )
  {
    VITAL_THROW( kv::metadata_exception,
                 "geographic sdcc mdarray does not have correct dimensions" );
  }

  auto const count = result.sizes.at( 1 );
  for( auto const i : kvr::iota( result.sizes.at( 0 ) ) )
  {
    for( auto const j : kvr::iota( count ) )
    {
      auto const int_element = int_value.elements.at( i * count + j );
      auto const& imap_params = geographic_sdcc_imap_params[ i ];
      auto const imap_element =
        int_to_imap( int_element, imap_params, result.element_size );
      result.elements.emplace_back( imap_element );
    }
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_1601_geographic_sdcc_format
::write_typed( klv_1303_mdap< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const& format = geographic_sdcc_internal_format;
  klv_1303_mdap< uint64_t > int_value =
  {
    value.sizes,
    {},
    value.element_size,
    value.apa,
    value.apa_params_length,
    value.imap_params };

  if( int_value.sizes.size() != 2 ||
      ( int_value.sizes.at( 0 ) != 3 && int_value.sizes.at( 0 ) != 6 ) )
  {
    VITAL_THROW( kv::metadata_exception,
                 "geographic sdcc mdarray does not have correct dimensions" );
  }

  auto const count = int_value.sizes.at( 1 );
  for( auto const i : kvr::iota< size_t >( int_value.sizes.at( 0 ) ) )
  {
    for( auto const j : kvr::iota( count ) )
    {
      auto const imap_element = value.elements.at( i * count + j );
      auto const& imap_params = geographic_sdcc_imap_params[ i ];
      auto const int_element =
        imap_to_int( imap_element, imap_params, int_value.element_size );
      int_value.elements.emplace_back( int_element );
    }
  }

  format.write_( int_value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_1601_geographic_sdcc_format
::length_of_typed( klv_1303_mdap< double > const& value ) const
{
  return geographic_sdcc_external_format.length_of_( value );
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1601_key()
{
  // From Section 6.2 of
  // https://gwg.nga.mil/misb/docs/standards/ST1601.1.pdf
  return { 0x060E2B34020B0101, 0x0E01030301000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_1601_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_1601_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_1601_VERSION ),
      std::make_shared< klv_uint_format >(),
      "Document Version",
      "Version number of the ST1601 document used to encode this metadata.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1601_ALGORITHM_NAME ),
      std::make_shared< klv_utf_8_format >(),
      "Algorithm Name",
      "Unique identifier for the algorithm used to geo-register the imagery.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1601_ALGORITHM_VERSION ),
      std::make_shared< klv_utf_8_format >(),
      "Algorithm Version",
      "Unique identifier for the specific version of the algorithm used.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1601_PIXEL_POINTS ),
      std::make_shared< klv_1303_mdap_format< klv_uint_format > >(),
      "Correspondence Points - Row / Column",
      "List of tie points represented in pixel space.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1601_GEOGRAPHIC_POINTS ),
      std::make_shared<
        klv_1303_mdap_format<
          klv_lengthless_format< klv_float_format > > >( 8 ),
      "Correspondence Points - Latitude / Longitude",
      "List of tie points represented in geographic space.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1601_SECOND_IMAGE_NAME ),
      std::make_shared< klv_utf_8_format >(),
      "Second Image Name",
      "Unique identifier for the second image used in the geo-registration "
      "process.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1601_ALGORITHM_CONFIG_ID ),
      std::make_shared< klv_uuid_format >(),
      "Algorithm Configuration Identifier",
      "Vendor-defined unique identifier for the parameters used with the "
      "geo-registration algorithm.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1601_ELEVATION ),
      std::make_shared<
        klv_1303_mdap_format<
          klv_lengthless_format< klv_float_format > > >( 4 ),
      "Correspondence Points - Elevation",
      "List of elevation values for the geographic tie points.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1601_PIXEL_SDCC ),
      std::make_shared< klv_1601_pixel_sdcc_format >(),
      "Correspondence Points SDCC - Row / Column",
      "Standard deviation and correlation coefficient values for the "
      "pixel-space tie points.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1601_GEOGRAPHIC_SDCC ),
      std::make_shared< klv_1601_geographic_sdcc_format >(),
      "Correspondence Points SDCC - Latitude / Longitude / Elevation",
      "Standard deviation and correlation coefficient values for the "
      "geographic-space tie points.",
      { 0, 1 } }, };

  return lookup;
}

// ----------------------------------------------------------------------------
klv_1601_local_set_format
::klv_1601_local_set_format()
  : klv_local_set_format{ klv_1601_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_1601_local_set_format
::description_() const
{
  return "ST1601 Geo-Registration LS";
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
