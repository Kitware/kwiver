// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1202 parser.

#include "klv_1202.h"

#include <arrows/klv/klv_1010.h>

#include <algorithm>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1202_tag tag )
{
  return os << klv_1202_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1202_transformation_type value )
{
  static std::string strings[ KLV_1202_TRANSFORMATION_TYPE_ENUM_END + 1 ] = {
    "Not Defined",
    "Chipping",
    "Child-Parent",
    "Pixel to Image Space",
    "Optical",
    "Unknown Transformation Type" };

  os << strings[ std::min( value, KLV_1202_TRANSFORMATION_TYPE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
klv_1202_local_set_format
::klv_1202_local_set_format()
  : klv_local_set_format{ klv_1202_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_1202_local_set_format
::description_() const
{
  return "ST1202 Generalized Transformation LS";
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1202_key()
{
  return { 0x060E2B34020B0101, 0x0E01030505000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_1202_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_1202_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010101, 0x0E01020281010000 },
      ENUM_AND_NAME( KLV_1202_X_NUMERATOR_X_FACTOR ),
      std::make_shared< klv_lengthless_format< klv_float_format > >( 4 ),
      "X Equation Numeration - X Factor",
      "Value A in Equation 1 of ST1202.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020281020000 },
      ENUM_AND_NAME( KLV_1202_X_NUMERATOR_Y_FACTOR ),
      std::make_shared< klv_lengthless_format< klv_float_format > >( 4 ),
      "X Equation Numeration - Y Factor",
      "Value B in Equation 1 of ST1202.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020281030000 },
      ENUM_AND_NAME( KLV_1202_X_NUMERATOR_CONSTANT ),
      std::make_shared< klv_lengthless_format< klv_float_format > >( 4 ),
      "X Equation Numeration - Constant",
      "Value C in Equation 1 of ST1202.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020281040000 },
      ENUM_AND_NAME( KLV_1202_Y_NUMERATOR_X_FACTOR ),
      std::make_shared< klv_lengthless_format< klv_float_format > >( 4 ),
      "Y Equation Numeration - X Factor",
      "Value D in Equation 2 of ST1202.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020281050000 },
      ENUM_AND_NAME( KLV_1202_Y_NUMERATOR_Y_FACTOR ),
      std::make_shared< klv_lengthless_format< klv_float_format > >( 4 ),
      "Y Equation Numeration - Y Factor",
      "Value E in Equation 2 of ST1202.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020281060000 },
      ENUM_AND_NAME( KLV_1202_Y_NUMERATOR_CONSTANT ),
      std::make_shared< klv_lengthless_format< klv_float_format > >( 4 ),
      "Y Equation Numeration - Constant",
      "Value F in Equation 2 of ST1202.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020281070000 },
      ENUM_AND_NAME( KLV_1202_DENOMINATOR_X_FACTOR ),
      std::make_shared< klv_lengthless_format< klv_float_format > >( 4 ),
      "Denominator - X Factor",
      "Value G in Equations 1 and 2 of ST1202.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020281080000 },
      ENUM_AND_NAME( KLV_1202_DENOMINATOR_Y_FACTOR ),
      std::make_shared< klv_lengthless_format< klv_float_format > >( 4 ),
      "Denominator - Y Factor",
      "Value H in Equations 1 and 2 of ST1202.",
      { 0, 1 } },
    { { 0x060E2B3402050101, 0x0E01030321000000 },
      ENUM_AND_NAME( KLV_1202_SDCC_FLP ),
      std::make_shared< klv_1010_sdcc_flp_format >(),
      "SDCC-FLP",
      "Standard Deviation and Correlation Coefficient Pack.",
      { 0, SIZE_MAX } },
    { { 0x060E2B3401010101, 0x0E01020505000000 },
      ENUM_AND_NAME( KLV_1202_VERSION ),
      std::make_shared< klv_uint_format >( 1 ),
      "Document Version",
      "Version number of MISB ST1202 document used to encode this metadata.",
      1 },
    { { 0x060E2B3401010101, 0x0E0102035F000000 },
      ENUM_AND_NAME( KLV_1202_TRANSFORMATION_TYPE ),
      std::make_shared< klv_1202_transformation_type_format >(),
      "Transformation Type",
      "Type of transformation encoded.",
      { 0, 1 } } };

  return lookup;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
