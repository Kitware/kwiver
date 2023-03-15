// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1107 parser.

#include "klv_1107.h"

#include <arrows/klv/klv_1010.h>
#include <arrows/klv/klv_1202.h>
#include <arrows/klv/klv_checksum.h>

#include <vital/util/interval.h>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
klv_lengthless_imap_format
sdcc_flp_sigma_imap( klv_lds_key key, size_t length )
{
  kv::interval< double > bounds{ 0.0, 0.0 };
  switch( key )
  {
    case KLV_1107_SENSOR_ABSOLUTE_AZIMUTH:
    case KLV_1107_SENSOR_ABSOLUTE_PITCH:
    case KLV_1107_SENSOR_ABSOLUTE_ROLL:
      bounds = { 0.0, 0.2 };
      break;

    case KLV_1107_SENSOR_ABSOLUTE_AZIMUTH_RATE:
    case KLV_1107_SENSOR_ABSOLUTE_PITCH_RATE:
    case KLV_1107_SENSOR_ABSOLUTE_ROLL_RATE:
    case KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_X:
    case KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_Y:
      bounds = { 0.0, 1.0 };
      break;

    case KLV_1107_BORESIGHT_DELTA_ANGLE_1:
    case KLV_1107_BORESIGHT_DELTA_ANGLE_2:
    case KLV_1107_BORESIGHT_DELTA_ANGLE_3:
      bounds = { 0.0, 2.0 };
      break;

    case KLV_1107_SENSOR_ECEF_VELOCITY_X:
    case KLV_1107_SENSOR_ECEF_VELOCITY_Y:
    case KLV_1107_SENSOR_ECEF_VELOCITY_Z:
      bounds = { 0.0, 70.0 };
      break;

    case KLV_1107_EFFECTIVE_FOCAL_LENGTH:
    case KLV_1107_EFFECTIVE_FOCAL_LENGTH_EXTENDED:
      bounds = { 0.0, 350.0 };

    case KLV_1107_SENSOR_ECEF_POSITION_X:
    case KLV_1107_SENSOR_ECEF_POSITION_Y:
    case KLV_1107_SENSOR_ECEF_POSITION_Z:
    case KLV_1107_BORESIGHT_OFFSET_DELTA_X:
    case KLV_1107_BORESIGHT_OFFSET_DELTA_Y:
    case KLV_1107_BORESIGHT_OFFSET_DELTA_Z:
    case KLV_1107_SLANT_RANGE:
      bounds = { 0.0, 650.0 };
      break;

    default:
      VITAL_THROW(
        kv::invalid_value,
        "sdcc_flp_sigma_imap(): ST1107 tag " + std::to_string( key ) +
        " does not support IMAP-encoded standard deviations" );
      break;
  }
  return { bounds, length };
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1107_tag tag )
{
  return os << klv_1107_traits_lookup().by_tag( tag ).tag();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1107_slant_range_pedigree value )
{
  static std::string strings[ klv_1107_SLANT_RANGE_PEDIGREE_ENUM_END + 1 ] = {
    "Other",
    "Measured",
    "Calculated",
    "Unknown Slant Range Pedigree" };

  os << strings[ std::min( value, klv_1107_SLANT_RANGE_PEDIGREE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
klv_1107_local_set_format
::klv_1107_local_set_format()
  : klv_local_set_format{ klv_1107_traits_lookup() },
    m_checksum_format{ { KLV_1107_CHECKSUM, 2 } }
{}

// ----------------------------------------------------------------------------
klv_checksum_packet_format const*
klv_1107_local_set_format
::packet_checksum_format() const
{
  return &m_checksum_format;
}

// ----------------------------------------------------------------------------
std::string
klv_1107_local_set_format
::description_() const
{
  return "ST1107 Metric Geopositioning LS";
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1107_key()
{
  return { 0x060E2B34020B0101, 0x0E01030322000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_1107_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_1107_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Tag",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ECEF_POSITION_X ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -1.0e9, 1.0e9 } ),
      "Sensor ECEF Position Component X",
      "Distance from the Earth's center of mass to the sensor reference point, "
      "along the geocentric axis which points towards the intersection of the "
      "equator and the IRM. Measured in meters.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ECEF_POSITION_Y ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -1.0e9, 1.0e9 } ),
      "Sensor ECEF Position Component Y",
      "Distance from the Earth's center of mass to the sensor reference point, "
      "along the cross product of the ECEF Z and X axes. This forms a "
      "right-handed coordinate system. Measured in meters.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ECEF_POSITION_Z ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -1.0e9, 1.0e9 } ),
      "Sensor ECEF Position Component Z",
      "Distance from the Earth's center of mass to the sensor reference point, "
      "along the geocentric axis which points towards the North Pole. Measured "
      "in meters.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ECEF_VELOCITY_X ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -25.0e3, 25.0e3 } ),
      "Sensor ECEF Velocity Component X",
      "Rate of change of the Sensor ECEF Position Component X. Measured in "
      "meters per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ECEF_VELOCITY_Y ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -25.0e3, 25.0e3 } ),
      "Sensor ECEF Velocity Component Y",
      "Rate of change of the Sensor ECEF Position Component Y. Measured in "
      "meters per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ECEF_VELOCITY_Z ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -25.0e3, 25.0e3 } ),
      "Sensor ECEF Velocity Component Z",
      "Rate of change of the Sensor ECEF Position Component Z. Measured in "
      "meters per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ABSOLUTE_AZIMUTH ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 2.0 } ),
      "Sensor Absolute Azimuth",
      "Angle from True North to the boresight vector projected onto the local "
      "horizontal plane, with a north-to-east rotation being positive. Measured in half-circles.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ABSOLUTE_PITCH ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -1.0, 1.0 } ),
      "Sensor Absolute Pitch",
      "Angle between the boresight vector and the local horizontal plane, with "
      "angles above the horizontal being positive. Measured in half-circles.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ABSOLUTE_ROLL ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -1.0, 1.0 } ),
      "Sensor Absolute Roll",
      "Angle between the boresight vector and the local horizontal plane, with "
      "clockwise rotations being positive. Measured in half-circles.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ABSOLUTE_AZIMUTH_RATE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -1.0, 1.0 } ),
      "Sensor Absolute Azimuth Rate",
      "Rate of change of the Sensor Absolute Azimuth. Measured in half-circles "
      "per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ABSOLUTE_PITCH_RATE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -1.0, 1.0 } ),
      "Sensor Absolute Pitch Rate",
      "Rate of change of the Sensor Absolute Pitch. Measured in half-circles "
      "per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SENSOR_ABSOLUTE_ROLL_RATE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -1.0, 1.0 } ),
      "Sensor Absolute Roll Rate",
      "Rate of change of the Sensor Absolute Roll. Measured in half-circles "
      "per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_BORESIGHT_OFFSET_DELTA_X ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -300.0, 300.0 } ),
      "Boresight Offset Delta X",
      "X component of the translation from the sensor reference point to the "
      "sensor perspective sensor. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_BORESIGHT_OFFSET_DELTA_Y ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -300.0, 300.0 } ),
      "Boresight Offset Delta Y",
      "Y component of the translation from the sensor reference point to the "
      "sensor perspective sensor. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_BORESIGHT_OFFSET_DELTA_Z ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -300.0, 300.0 } ),
      "Boresight Offset Delta Z",
      "Z component of the translation from the sensor reference point to the "
      "sensor perspective sensor. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_BORESIGHT_DELTA_ANGLE_1 ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -0.25, 0.25 } ),
      "Boresight Delta Angle 1",
      "Rotation around the x axis to align the sensor reference axes with the "
      "sensor principal axes. This rotation is applied third. Measured in "
      "half-circles.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_BORESIGHT_DELTA_ANGLE_2 ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -0.25, 0.25 } ),
      "Boresight Delta Angle 2",
      "Rotation around the y axis to align the sensor reference axes with the "
      "sensor principal axes. This rotation is applied second. Measured in "
      "half-circles.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_BORESIGHT_DELTA_ANGLE_3 ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -0.25, 0.25 } ),
      "Boresight Delta Angle 3",
      "Rotation around the z axis to align the sensor reference axes with the "
      "sensor principal axes. This rotation is applied first. Measured in "
      "half-circles.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_Y ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -25.0, 25.0 } ),
      "Focal Plane Principal Point Offset Y",
      "Vertical component of the translation on the focal plane from the "
      "center of the frame to the principal point. Measured in millimeters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_X ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ -25.0, 25.0 } ),
      "Focal Plane Principal Point Offset X",
      "Horizontal component of the translation on the focal plane from the "
      "center of the frame to the principal point. Measured in millimeters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_EFFECTIVE_FOCAL_LENGTH ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 10000.0 } ),
      "Sensor Calibrated / Effective Focal Length",
      "Distance from perspective center to the detector array. Measured in "
      "millimeters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_RADIAL_DISTORTION_CONSTANT ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Radial Distortion Constant",
      "Coefficient for the linear (r^1) term of the radial distortion "
      "equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_RADIAL_DISTORTION_PARAMETER_1 ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Radial Distortion Parameter 1",
      "Coefficient for the cubic (r^3) term of the radial distortion "
      "equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_RADIAL_DISTORTION_PARAMETER_2 ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Radial Distortion Parameter 2",
      "Coefficient for the quintic (r^5) term of the radial distortion "
      "equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_RADIAL_DISTORTION_PARAMETER_3 ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Radial Distortion Parameter 3",
      "Coefficient for the septic (r^7) term of the radial distortion "
      "equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_1 ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Tangential / Decentering Parameter 1",
      "Parameter P_1 in the tangential-decentering distortion equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_2 ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Tangential / Decentering Parameter 2",
      "Parameter P_2 in the tangential-decentering distortion equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_3 ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Tangential / Decentering Parameter 3",
      "Parameter P_3 in the tangential-decentering distortion equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_DIFFERENTIAL_SCALE_AFFINE_PARAMETER ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Differential Scale Affine Parameter",
      "Parameter b_1 in the affine correction equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SKEWNESS_AFFINE_PARAMETER ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Skewness Affine Parameter",
      "Parameter b_2 in the affine correction equation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SLANT_RANGE ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Slant Range",
      "Distance from the perspective center to a point on the ground in the "
      "scene. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SDCC_FLP ),
      std::make_shared< klv_1010_sdcc_flp_format >( &sdcc_flp_sigma_imap ),
      "Standard Deviation and Correlation Coefficient Floating-Length Pack",
      "MISB ST1010 SDCC-FLP. Contains standard deviation and correlation "
      "coefficient information about the measured entities in this local set.",
      { 1, SIZE_MAX } },
    { {},
      ENUM_AND_NAME( KLV_1107_GENERALIZED_TRANSFORMATION_LOCAL_SET ),
      std::make_shared< klv_1202_local_set_format >(),
      "Generalized Transformation Local Set",
      "MISB ST1202 Generalized Transformation Local Set. Relates the virtual "
      "image coordinate system to the distorted image coordinate system.",
      { 0, 4 } },
    { {},
      ENUM_AND_NAME( KLV_1107_IMAGE_ROWS ),
      std::make_shared< klv_uint_format >( 2 ),
      "Image Rows",
      "Vertical span of the source image in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_IMAGE_COLUMNS ),
      std::make_shared< klv_uint_format >( 2 ),
      "Image Columns",
      "Horizontal span of the source image in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_PIXEL_SIZE_X ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 1.0e-4, 1.0e-1 } ),
      "Pixel Size X",
      "Width of each pixel. Measured in millimeters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_PIXEL_SIZE_Y ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 1.0e-4, 1.0e-1 } ),
      "Pixel Size Y",
      "Height of each pixel. Measured in millimeters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SLANT_RANGE_PEDIGREE ),
      std::make_shared< klv_1107_slant_range_pedigree_format >( 1 ),
      "Slant Range Pedigree",
      "Method by which the slant range value was determined.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_LINE_COORDINATE ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Line Coordinate",
      "Vertical coordinate of the slant range relative to the top of the "
      "uppermost pixel. Measured in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_SAMPLE_COORDINATE ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Sample Coordinate",
      "Horizontal coordinate of the slant range relative to the left side of "
      "the leftmost pixel. Measured in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_LRF_DIVERGENCE ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "LRF Divergence",
      "Divergence of the laser range finder used to measure slant range. "
      "Measured in radians.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_RADIAL_DISTORTION_VALID_RANGE ),
      std::make_shared< klv_lengthless_float_format >( 4 ),
      "Radial Distortion Valid Range",
      "Radial distance from the principal point for which the distortion "
      "estimation equation is acceptably accurate. Measured in millimeters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_PRECISION_TIMESTAMP ),
      std::make_shared< klv_uint_format >( 8 ),
      "Precision Timestamp",
      "MISP precision timestamp expressed in microseconds since the UNIX "
      "Epoch.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1107_DOCUMENT_VERSION ),
      std::make_shared< klv_ber_oid_format >(),
      "Document Version",
      "Version number of MISB ST1107 used as the source standard when encoding "
      "this local set.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_1107_CHECKSUM ),
      std::make_shared< klv_uint_format >( 2 ),
      "Checksum",
      "CRC-16-CCITT checksum used to detect errors within a ST1107 packet.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_1107_LEAP_SECONDS ),
      std::make_shared< klv_sint_format >(),
      "Leap Seconds",
      "Current number of leap seconds, to facilitate conversion between MISP "
      "and UTC time systems.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_1107_EFFECTIVE_FOCAL_LENGTH_EXTENDED ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 100000.0 } ),
      "Sensor Calibrated / Effective Focal Length Extended",
      "Distance from perspective center to the detector array. Measured in "
      "millimeters.",
      { 0, 1 } }, };

  return lookup;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
