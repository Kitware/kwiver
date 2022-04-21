// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 VTrackItem local set parser.

#include "klv_0903_vtrackitem_pack.h"

#include <arrows/klv/klv_0903_location_pack.h>
#include <arrows/klv/klv_0903_vchip_set.h>
#include <arrows/klv/klv_0903_vfeature_set.h>
#include <arrows/klv/klv_0903_vmask_set.h>
#include <arrows/klv/klv_0903_vobject_set.h>
#include <arrows/klv/klv_0903_vtarget_pack.h>
#include <arrows/klv/klv_1204.h>
#include <arrows/klv/klv_series.hpp>
#include <arrows/klv/klv_util.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vtrackitem_pack_tag tag )
{
  return os << klv_0903_vtrackitem_pack_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vtrackitem_pack const& value )
{
  os    << "{ "
        << "id: " << value.id << ", "
        << "set: " << value.set
        << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0903_vtrackitem_pack,
  &klv_0903_vtrackitem_pack::id,
  &klv_0903_vtrackitem_pack::set
  )

// ----------------------------------------------------------------------------
klv_0903_vtrackitem_pack_format
::klv_0903_vtrackitem_pack_format()
  : klv_data_format_< klv_0903_vtrackitem_pack >{ 0 } {}

// ----------------------------------------------------------------------------
std::string
klv_0903_vtrackitem_pack_format
::description() const
{
  return "vtrackitem pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0903_vtrackitem_pack
klv_0903_vtrackitem_pack_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_0903_vtrackitem_pack result;
  result.id = klv_read_ber_oid< size_t >( data, tracker.remaining() );
  result.set =
    klv_0903_vtrackitem_local_set_format()
    .read( data, tracker.remaining() ).get< klv_local_set >();
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0903_vtrackitem_pack_format
::write_typed( klv_0903_vtrackitem_pack const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_ber_oid( value.id, data, tracker.remaining() );
  klv_0903_vtrackitem_local_set_format()
    .write( value.set, data, tracker.remaining() );
}

// ----------------------------------------------------------------------------
size_t
klv_0903_vtrackitem_pack_format
::length_of_typed( klv_0903_vtrackitem_pack const& value ) const
{
  return klv_ber_oid_length( value.id ) +
         klv_0903_vtrackitem_local_set_format().length_of( value.set );
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_vtrackitem_pack_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_TIMESTAMP ),
      std::make_shared< klv_uint_format >( 8 ),
      "Target Timestamp",
      "Microseconds since January 1st, 1970.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_CENTROID ),
      std::make_shared< klv_uint_format >(),
      "Target Centroid",
      "Index of the centroid pixel. Uses the equation (row - 1) * width + "
      "column, where row and column are 1-indexed.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_CENTROID_ROW ),
      std::make_shared< klv_uint_format >(),
      "Centroid Pixel Row",
      "Row of the target centroid pixel, with 1 being the topmost row.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_CENTROID_COLUMN ),
      std::make_shared< klv_uint_format >(),
      "Centroid Pixel Column",
      "Column of the target centroid pixel, with 1 being the leftmost column.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_BOUNDARY_TOP_LEFT ),
      std::make_shared< klv_uint_format >(),
      "Boundary Top Left",
      "Index of the top-left corner pixel of the target bounding box. Uses "
      "the equation (row - 1) * width + column, where row and column are "
      "1-indexed.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_BOUNDARY_BOTTOM_RIGHT ),
      std::make_shared< klv_uint_format >(),
      "Boundary Bottom Right",
      "Index of the bottom-right corner pixel of the target bounding box. "
      "Uses the equation (row - 1) * width + column, where row and column are "
      "1-indexed.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_PRIORITY ),
      std::make_shared< klv_uint_format >( 1 ),
      "Target Priority",
      "Provides downstream systems a means to cull targets. Lower numbers are "
      "higher priority.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_CONFIDENCE_LEVEL ),
      std::make_shared< klv_uint_format >( 1 ),
      "Target Confidence Level",
      "Confidence level, as a percentage, of the target detection.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_HISTORY ),
      std::make_shared< klv_uint_format >(),
      "Target History",
      "Number of times a target has previously been detected.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_PERCENT_PIXELS ),
      std::make_shared< klv_uint_format >( 1 ),
      "Percentage of Target Pixels",
      "Integer percentage of the pixels in the image classified as target "
      "pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_COLOR ),
      std::make_shared< klv_uint_format >( 3 ),
      "Target Color",
      "Dominant color of the target, expressed as three RGB bytes.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_INTENSITY ),
      std::make_shared< klv_uint_format >(),
      "Target Intensity",
      "Dominant intensity of the target, expressed as a single integer using "
      "up to 24 bits.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_LOCATION ),
      std::make_shared< klv_0903_location_pack_format >(),
      "Target Location",
      "Geographical position of target, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_BOUNDARY_SERIES ),
      std::make_shared< klv_0903_location_series_format >(),
      "Target Boundary Series",
      "An arbitrary number of geospatial vertices defining the boundary "
      "around an area or volume of interest",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_VELOCITY ),
      std::make_shared< klv_0903_velocity_pack_format >(),
      "Velocity",
      "Velocity of the entity at the time of last observation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_ACCELERATION ),
      std::make_shared< klv_0903_acceleration_pack_format >(),
      "Acceleration",
      "Acceleration of the entity at the time of last observation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_FPA_INDEX ),
      std::make_shared< klv_0903_fpa_index_format >(),
      "FPA Index Pack",
      "Index of Focal Plane Array in which detection of the target occurs.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_FRAME_NUMBER ),
      std::make_shared< klv_uint_format >(),
      "Frame Number",
      "Frame number identifying detected targets.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_MIIS_ID ),
      std::make_shared< klv_1204_miis_id_format >(),
      "MIIS ID",
      "A Motion Imagery Identification System Core Identifier conformant with "
      "MISB ST 1204.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_FRAME_WIDTH ),
      std::make_shared< klv_uint_format >(),
      "Frame Width",
      "Width of the Motion Imagery frame in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_FRAME_HEIGHT ),
      std::make_shared< klv_uint_format >(),
      "Frame Height",
      "Height of the Motion Imagery frame in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_HORIZONTAL_FOV ),
      std::make_shared< klv_imap_format >( 0.0, 180.0 ),
      "VMTI Horizontal FOV",
      "Horizonal field of view of sensor input to the VMTI process.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_VERTICAL_FOV ),
      std::make_shared< klv_imap_format >( 0.0, 180.0 ),
      "VMTI Vertical FOV",
      "Vertical field of view of sensor input to the VMTI process.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_MI_URL ),
      std::make_shared< klv_string_format >(),
      "Motion Imagery URL",
      "A URL for the Motion Imagery, conformant with IETF RFC 3986.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_VMASK ),
      std::make_shared< klv_0903_vmask_local_set_format >(),
      "Target Mask",
      "Outline of the detected target, in the form of a bitmask or a polygon.",
      { 0, 1 },
      &klv_0903_vmask_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_VOBJECT ),
      std::make_shared< klv_0903_vobject_local_set_format >(),
      "Target Object",
      "Class or type of the target to an arbitrary level of detail.",
      { 0, 1 },
      &klv_0903_vobject_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_VFEATURE ),
      std::make_shared< klv_0903_vfeature_local_set_format >(),
      "Target Features",
      "Data which describes the target or features of the target, in varying "
      "forms.",
      { 0, 1 },
      &klv_0903_vfeature_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_VCHIP ),
      std::make_shared< klv_0903_vchip_local_set_format >(),
      "Target Chip",
      "Embedded image chip of the target, or URI linking to it.",
      { 0, 1 },
      &klv_0903_vchip_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_VCHIP_SERIES ),
      std::make_shared< klv_0903_vchip_series_format >(),
      "Chip Series",
      "Series of embedded image chips of the target, or URIs linking to them.",
      { 0, 1 },
      &klv_0903_vchip_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKITEM_VOBJECT_SERIES ),
      std::make_shared< klv_0903_vobject_series_format >(),
      "Object Series",
      "Series of object classes describing the target.",
      { 0, 1 },
      &klv_0903_vobject_set_traits_lookup() } };

  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_vtrackitem_local_set_format
::klv_0903_vtrackitem_local_set_format()
  : klv_local_set_format{ klv_0903_vtrackitem_pack_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_vtrackitem_local_set_format
::description() const
{
  return "vtrackitem local set of " + length_description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
