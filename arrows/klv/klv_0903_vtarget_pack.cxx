// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 VTrack local set parser.

#include "klv_0903_vtarget_pack.h"

#include <arrows/klv/klv_0903_location_pack.h>
#include <arrows/klv/klv_0903_vchip_set.h>
#include <arrows/klv/klv_0903_vfeature_set.h>
#include <arrows/klv/klv_0903_vmask_set.h>
#include <arrows/klv/klv_0903_vobject_set.h>
#include <arrows/klv/klv_0903_vtracker_set.h>
#include <arrows/klv/klv_series.hpp>
#include <arrows/klv/klv_util.h>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vtarget_pack_tag tag )
{
  return os << klv_0903_vtarget_pack_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vtarget_pack const& value )
{
  os    << "{ "
        << "id: " << value.id << ", "
        << "set: ";
  klv_0903_vtarget_local_set_format().print( os, value.set );
  os    << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0903_vtarget_pack,
  &klv_0903_vtarget_pack::id,
  &klv_0903_vtarget_pack::set
  )

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_fpa_index const& value )
{
  os    << "{ "
        << "row: " << static_cast< unsigned int >( value.row ) << ", "
        << "column: " << static_cast< unsigned int >( value.column )
        << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0903_fpa_index,
  &klv_0903_fpa_index::row,
  &klv_0903_fpa_index::column
  )

// ----------------------------------------------------------------------------
klv_0903_fpa_index_format
::klv_0903_fpa_index_format()
  : klv_data_format_< klv_0903_fpa_index >{ 2 } {}

// ----------------------------------------------------------------------------
std::string
klv_0903_fpa_index_format
::description() const
{
  return "fpa index pack of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
klv_0903_fpa_index
klv_0903_fpa_index_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  if( length < 2 )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "reading fpa index pack overflows data buffer" );
  }

  klv_0903_fpa_index result;
  result.row = klv_read_int< uint8_t >( data, 1 );
  result.column = klv_read_int< uint8_t >( data, 1 );
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0903_fpa_index_format
::write_typed( klv_0903_fpa_index const& value,
               klv_write_iter_t& data, size_t length ) const
{
  if( length < 2 )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "writing fpa index pack overflows data buffer" );
  }
  klv_write_int( value.row, data, 1 );
  klv_write_int( value.column, data, 1 );
}

// ----------------------------------------------------------------------------
size_t
klv_0903_fpa_index_format
::length_of_typed( VITAL_UNUSED klv_0903_fpa_index const& value ) const
{
  return 2;
}

// ----------------------------------------------------------------------------
klv_0903_vtarget_pack_format
::klv_0903_vtarget_pack_format()
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_vtarget_pack_format
::description() const
{
  return "vtarget pack of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
klv_0903_vtarget_pack
klv_0903_vtarget_pack_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_0903_vtarget_pack result;
  result.id = klv_read_ber_oid< size_t >( data, tracker.remaining() );
  result.set =
    klv_0903_vtarget_local_set_format()
    .read( data, tracker.remaining() ).get< klv_local_set >();
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0903_vtarget_pack_format
::write_typed( klv_0903_vtarget_pack const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_ber_oid( value.id, data, tracker.remaining() );
  klv_0903_vtarget_local_set_format()
    .write( value.set, data, tracker.remaining() );
}

// ----------------------------------------------------------------------------
size_t
klv_0903_vtarget_pack_format
::length_of_typed( klv_0903_vtarget_pack const& value ) const
{
  return klv_ber_oid_length( value.id ) +
         klv_0903_vtarget_local_set_format().length_of( value.set );
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_vtarget_pack_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_CENTROID ),
      std::make_shared< klv_uint_format >(),
      "Target Centroid",
      "Index of the centroid pixel. Uses the equation (row - 1) * width + "
      "column, where row and column are 1-indexed.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_BOUNDARY_TOP_LEFT ),
      std::make_shared< klv_uint_format >(),
      "Boundary Top Left",
      "Index of the top-left corner pixel of the target bounding box. Uses "
      "the equation (row - 1) * width + column, where row and column are "
      "1-indexed.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT ),
      std::make_shared< klv_uint_format >(),
      "Boundary Bottom Right",
      "Index of the bottom-right corner pixel of the target bounding box. "
      "Uses the equation (row - 1) * width + column, where row and column are 1-indexed.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_PRIORITY ),
      std::make_shared< klv_uint_format >( 1 ),
      "Target Priority",
      "Provides downstream systems a means to cull targets. Lower numbers are "
      "higher priority.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_CONFIDENCE_LEVEL ),
      std::make_shared< klv_uint_format >( 1 ),
      "Target Confidence Level",
      "Confidence level, as a percentage, of the target detection.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_HISTORY ),
      std::make_shared< klv_uint_format >(),
      "Target History",
      "Number of times a target has previously been detected.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_PERCENT_PIXELS ),
      std::make_shared< klv_uint_format >( 1 ),
      "Percentage of Target Pixels",
      "Integer percentage of the pixels in the image classified as target "
      "pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_COLOR ),
      std::make_shared< klv_uint_format >( 3 ),
      "Target Color",
      "Dominant color of the target, expressed as three RGB bytes.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_INTENSITY ),
      std::make_shared< klv_uint_format >(),
      "Target Intensity",
      "Dominant intensity of the target, expressed as a single integer using "
      "up to 24 bits.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_LOCATION_OFFSET_LATITUDE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -19.2, 19.2 } ),
      "Target Location Offset Latitude",
      "Latitude offset for target from Frame Center Latitude, based on WGS84 "
      "ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_LOCATION_OFFSET_LONGITUDE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -19.2, 19.2 } ),
      "Target Location Offset Longitude",
      "Longitude offset for target from Frame Center Longitude, based on "
      "WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_LOCATION_ELLIPSOID_HEIGHT ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -900.0, 19000.0 } ),
      "Target Height Above Ellipsoid",
      "Height of the target in meters above the WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_BOUNDARY_TOP_LEFT_LATITUDE_OFFSET ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -19.2, 19.2 } ),
      "Boundary Top Left Latitude Offset",
      "Latitude offset for the top left corner of the bounding box from Frame "
      "Center Latitude, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_BOUNDARY_TOP_LEFT_LONGITUDE_OFFSET ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -19.2, 19.2 } ),
      "Boundary Top Left Longitude Offset",
      "Longitude offset for the top left corner of the bounding box from Frame "
      "Center Longitude, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT_LATITUDE_OFFSET ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -19.2, 19.2 } ),
      "Boundary Bottom Right Latitude Offset",
      "Latitude offset for the bottom right corner of the bounding box from "
      "Frame Center Latitude, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT_LONGITUDE_OFFSET ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -19.2, 19.2 } ),
      "Boundary Bottom Right Longitude Offset",
      "Longitude offset for the bottom right corner of the bounding box from "
      "Frame Center Longitude, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_LOCATION ),
      std::make_shared< klv_0903_location_pack_format >(),
      "Target Location",
      "Geographical position of target, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_BOUNDARY_SERIES ),
      std::make_shared< klv_0903_location_series_format >(),
      "Target Boundary Series",
      "An arbitrary number of geospatial vertices defining the boundary "
      "around an area or volume of interest",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_CENTROID_ROW ),
      std::make_shared< klv_uint_format >(),
      "Centroid Pixel Row",
      "Row of the target centroid pixel, with 1 being the topmost row.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_CENTROID_COLUMN ),
      std::make_shared< klv_uint_format >(),
      "Centroid Pixel Column",
      "Column of the target centroid pixel, with 1 being the leftmost column.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_FPA_INDEX ),
      std::make_shared< klv_0903_fpa_index_format >(),
      "FPA Index Pack",
      "Index of Focal Plane Array in which detection of the target occurs.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_ALGORITHM_ID ),
      std::make_shared< klv_uint_format >(),
      "Algorithm ID",
      "Id number of algorithm used to detect the target.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_VMASK ),
      std::make_shared< klv_0903_vmask_local_set_format >(),
      "Target Mask",
      "Outline of the detected target, in the form of a bitmask or a polygon.",
      { 0, 1 },
      &klv_0903_vmask_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_VOBJECT ),
      std::make_shared< klv_0903_vobject_local_set_format >(),
      "Target Object",
      "Class or type of the target to an arbitrary level of detail.",
      { 0, 1 },
      &klv_0903_vobject_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_VFEATURE ),
      std::make_shared< klv_0903_vfeature_local_set_format >(),
      "Target Features",
      "Data which describes the target or features of the target, in varying "
      "forms.",
      { 0, 1 },
      &klv_0903_vfeature_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_VTRACKER ),
      std::make_shared< klv_0903_vtracker_local_set_format >(),
      "Target Tracker",
      "Contains ancillary spatial and temporal information to assist in "
      "tracking the target.",
      { 0, 1 },
      &klv_0903_vtracker_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_VCHIP ),
      std::make_shared< klv_0903_vchip_local_set_format >(),
      "Target Chip",
      "Embedded image chip of the target, or URI linking to it.",
      { 0, 1 },
      &klv_0903_vchip_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_VCHIP_SERIES ),
      std::make_shared< klv_0903_vchip_series_format >(),
      "Chip Series",
      "Series of embedded image chips of the target, or URIs linking to them.",
      { 0, 1 },
      &klv_0903_vchip_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_VOBJECT_SERIES ),
      std::make_shared< klv_0903_vobject_series_format >(),
      "Object Series",
      "Series of object classes describing the target.",
      { 0, 1 },
      &klv_0903_vobject_set_traits_lookup() } };
  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_vtarget_local_set_format
::klv_0903_vtarget_local_set_format()
  : klv_local_set_format{ klv_0903_vtarget_pack_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_vtarget_local_set_format
::description() const
{
  return "vtarget local set of " + m_length_constraints.description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
