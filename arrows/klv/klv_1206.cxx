// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1206 parser.

#include "klv_1206.h"

#include <arrows/klv/klv_1303.hpp>
#include <arrows/klv/klv_util.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1206_tag tag )
{
  return os << klv_1206_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1206_image_plane value )
{
  static std::string const strings[ KLV_1206_IMAGE_PLANE_ENUM_END + 1 ] = {
    "Slant",
    "Ground",
    "Other",
    "Unknown Image Plane" };

  return os << strings[ std::min( value, KLV_1206_IMAGE_PLANE_ENUM_END ) ];
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1206_look_direction value )
{
  static std::string const strings[ KLV_1206_LOOK_DIRECTION_ENUM_END + 1 ] = {
    "Left",
    "Right",
    "Unknown Look Direction" };

  return os << strings[ std::min( value, KLV_1206_LOOK_DIRECTION_ENUM_END ) ];
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1206_key()
{
  return { 0x060E2B34020B0101, 0x0E0103030D000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_1206_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_1206_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010101, 0x0E0101033D000000 },
      ENUM_AND_NAME( KLV_1206_GRAZING_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 90.0 }, 2 ),
      "Grazing Angle",
      "Angle between the line-of-sight vector from the scene reference point "
      "to the sensor and the ground plane at the reference point. Measured in "
      "degrees.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D010000 },
      ENUM_AND_NAME( KLV_1206_GROUND_PLANE_SQUINT_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -90.0, 90.0 }, 2 ),
      "Ground Plane Squint Angle",
      "Angle between the ground track vector and the radar's line-of-sight "
      "vector, projected onto the ground plane. Measured in degrees.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101033D020000 },
      ENUM_AND_NAME( KLV_1206_LOOK_DIRECTION ),
      std::make_shared< klv_1206_look_direction_format >(),
      "Look Direction",
      "Side of the imaging platform from which the imagery is collected, "
      "relative to the velocity vector.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101033D030000 },
      ENUM_AND_NAME( KLV_1206_IMAGE_PLANE ),
      std::make_shared< klv_1206_image_plane_format >(),
      "Image Plane",
      "Plane in which the SAR images were taken.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D040000 },
      ENUM_AND_NAME( KLV_1206_RANGE_RESOLUTION ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 1.0e6 }, 4 ),
      "Range Resolution",
      "Minimum distance at which two objects in close proximity in range may "
      "be resolved from one another. Measured in meters.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D050000 },
      ENUM_AND_NAME( KLV_1206_CROSS_RANGE_RESOLUTION ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 1.0e6 }, 4 ),
      "Cross-Range Resolution",
      "Minimum distance at which two objects in close proximity in cross "
      "range may be resolved from one another. Measured in meters.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D060000 },
      ENUM_AND_NAME( KLV_1206_RANGE_IMAGE_PLANE_PIXEL_SIZE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 1.0e6 }, 4 ),
      "Range Image Plane Pixel Size",
      "Pixel size in the range direction. Measured in meters.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D070000 },
      ENUM_AND_NAME( KLV_1206_CROSS_RANGE_IMAGE_PLANE_PIXEL_SIZE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 1.0e6 }, 4 ),
      "Cross-Range Image Plane Pixel Size",
      "Pixel size in the cross-range direction. Measured in meters.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020206000000 },
      ENUM_AND_NAME( KLV_1206_IMAGE_ROWS ),
      std::make_shared< klv_uint_format >( 2 ),
      "Image Rows",
      "Height of the image in pixels.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020207000000 },
      ENUM_AND_NAME( KLV_1206_IMAGE_COLUMNS ),
      std::make_shared< klv_uint_format >( 2 ),
      "Image Columns",
      "Width of the image in pixels.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D080000 },
      ENUM_AND_NAME( KLV_1206_RANGE_DIRECTION_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 360.0 }, 2 ),
      "Range Direction Angle",
      "Direction of the range vector relative to true north. Measured "
      "clockwise in degrees",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D090000 },
      ENUM_AND_NAME( KLV_1206_TRUE_NORTH ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 360.0 }, 2 ),
      "True North",
      "Direction of true north relative to the top edge of the image. "
      "Measured clockwise in degrees.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D0A0000 },
      ENUM_AND_NAME( KLV_1206_RANGE_LAYOVER_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 360.0 }, 2 ),
      "Range Layover Angle",
      "Direction perpendicular to the sensor ground track angle at the "
      "aperature center relative to true north. Measured clockwise in "
      "degrees.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D0B0000 },
      ENUM_AND_NAME( KLV_1206_GROUND_APERTURE_ANGULAR_EXTENT ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 90.0 }, 2 ),
      "Ground Aperture Angular Extent",
      "Angle swept in cross-range as the sensor traverses the synthetic "
      "aperture used to generate a SAR image. Measured in degrees.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D0C0000 },
      ENUM_AND_NAME( KLV_1206_APERTURE_DURATION ),
      std::make_shared< klv_uint_format >( 4 ),
      "Aperture Duration",
      "Length of the coherent processing period or the interval the radar "
      "beam illuminates the scene. Measured in microseconds.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D0D0000 },
      ENUM_AND_NAME( KLV_1206_GROUND_TRACK_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 360.0 }, 2 ),
      "Ground Track Angle",
      "Heading of the scene reference point over the ground relative to true "
      "north. Measured clockwise in degrees.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D0E0000 },
      ENUM_AND_NAME( KLV_1206_MINIMUM_DETECTABLE_VELOCITY ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 100.0 }, 2 ),
      "Minimum Detectable Velocity",
      "Radial velocity when a traget located at the antenna beam's "
      "cross-range center line transcends from endo-clutter to exo-clutter. "
      "Measured in meters per second.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D0F0000 },
      ENUM_AND_NAME( KLV_1206_TRUE_PULSE_REPETITION_FREQUENCY ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 1.0e6 }, 4 ),
      "True Pulse Repetition Frequency",
      "Time interval between successively transmitted pulses. Measured in "
      "microseconds.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D100000 },
      ENUM_AND_NAME( KLV_1206_PULSE_REPETITION_FREQUENCY_SCALE_FACTOR ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 1.0 }, 2 ),
      "Pulse Repetition Frequency Scale Factor",
      "Scale factor to calculate effective pulse repetition frequency from "
      "the true value.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D110000 },
      ENUM_AND_NAME( KLV_1206_TRANSMIT_RF_CENTER_FREQUENCY ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 1.0e12 }, 4 ),
      "Transmit RF Center Frequency",
      "Center frequency of the RF band when linear FM waveforms are employed. "
      "Measured in Hertz.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D120000 },
      ENUM_AND_NAME( KLV_1206_TRANSMIT_RF_BANDWIDTH ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 1.0e11 }, 4 ),
      "Transmit RF Bandwidth",
      "Difference between minimum and maximum transmit frequencies for a "
      "single or sequence of waveforms, if applicable. Measured in Hertz.",
      { 0, 1 } },
    { { 0x060E2B3402050101, 0x0E01030306000000 },
      ENUM_AND_NAME( KLV_1206_RADAR_CROSS_SECTION_SCALE_FACTOR_POLYNOMIAL ),
      std::make_shared<
        klv_1303_mdap_format<
          klv_lengthless_format< klv_imap_format > > >(
            vital::interval< double >{ 0.0, 1.0e6 }, 4 ),
      "Radar Cross Section Scale Factor Polynomial",
      "Two-dimensional array of polynomial coefficients used to determine the "
      "radar cross-section for a pixel. See MISB ST1206 for an explanation of "
      "the equation used.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D140000 },
      ENUM_AND_NAME( KLV_1206_REFERENCE_FRAME_PRECISION_TIMESTAMP ),
      std::make_shared< klv_uint_format >( 8 ),
      "Reference Frame Precision Timestamp",
      "For the reference frame: MISP precision timestamp. Measured in "
      "microseconds since January 1, 1970.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D150000 },
      ENUM_AND_NAME( KLV_1206_REFERENCE_FRAME_GRAZING_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 90.0 }, 2 ),
      "Reference Frame Grazing Angle",
      "For the reference frame: Angle between the line-of-sight vector from "
      "the scene reference point to the sensor and the ground plane at the "
      "reference point. Measured in degrees.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D160000 },
      ENUM_AND_NAME( KLV_1206_REFERENCE_FRAME_GROUND_PLANE_SQUINT_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -90.0, 90.0 }, 2 ),
      "Reference Frame Groud Plane Squint Angle",
      "For the reference frame: Angle between the ground track vector and the "
      "radar's line-of-sight vector, projected onto the ground plane. "
      "Measured in degrees.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D170000 },
      ENUM_AND_NAME( KLV_1206_REFERENCE_FRAME_RANGE_DIRECTION_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 360.0 }, 2 ),
      "Reference Frame Range Direction Angle",
      "For the reference frame: Direction of the range vector relative to "
      "true north. Measured clockwise in degrees",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033D180000 },
      ENUM_AND_NAME( KLV_1206_REFERENCE_FRAME_RANGE_LAYOVER_ANGLE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 360.0 }, 2 ),
      "Reference Frame Range Layover Angle",
      "For the reference frame: Direction perpendicular to the sensor ground "
      "track angle at the aperature center relative to true north. Measured "
      "clockwise in degrees.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020505000000 },
      ENUM_AND_NAME( KLV_1206_DOCUMENT_VERSION ),
      std::make_shared< klv_uint_format >( 1 ),
      "Document Version",
      "Version of MISB ST1206 used to encode the SAR metadata.",
      1 } };

  return lookup;
}

// ----------------------------------------------------------------------------
klv_1206_local_set_format
::klv_1206_local_set_format()
  : klv_local_set_format{ klv_1206_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_1206_local_set_format
::description() const
{
  return
    "SAR motion imagery local set of " + m_length_constraints.description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
