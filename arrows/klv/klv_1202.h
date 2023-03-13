// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1202 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1202_H_
#define KWIVER_ARROWS_KLV_KLV_1202_H_

#include <arrows/klv/klv_set.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1202_tag : klv_lds_key
{
  KLV_1202_UNKNOWN              = 0,
  KLV_1202_X_NUMERATOR_X_FACTOR = 1,
  KLV_1202_X_NUMERATOR_Y_FACTOR = 2,
  KLV_1202_X_NUMERATOR_CONSTANT = 3,
  KLV_1202_Y_NUMERATOR_X_FACTOR = 4,
  KLV_1202_Y_NUMERATOR_Y_FACTOR = 5,
  KLV_1202_Y_NUMERATOR_CONSTANT = 6,
  KLV_1202_DENOMINATOR_X_FACTOR = 7,
  KLV_1202_DENOMINATOR_Y_FACTOR = 8,
  KLV_1202_SDCC_FLP             = 9,
  KLV_1202_VERSION              = 10,
  KLV_1202_TRANSFORMATION_TYPE  = 11,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1202_tag tag );

// ----------------------------------------------------------------------------
enum klv_1202_transformation_type : uint8_t
{
  KLV_1202_TRANSFORMATION_TYPE_UNDEFINED            = 0,
  KLV_1202_TRANSFORMATION_TYPE_CHIPPING             = 1,
  KLV_1202_TRANSFORMATION_TYPE_CHILD_PARENT         = 2,
  KLV_1202_TRANSFORMATION_TYPE_PIXEL_TO_IMAGE_SPACE = 3,
  KLV_1202_TRANSFORMATION_TYPE_OPTICAL              = 4,
  KLV_1202_TRANSFORMATION_TYPE_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1202 transformation enumeration.
using klv_1202_transformation_type_format =
  klv_enum_format< klv_1202_transformation_type >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1202_transformation_type value );

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1202 generalized transformation local set.
class KWIVER_ALGO_KLV_EXPORT klv_1202_local_set_format
  : public klv_local_set_format
{
public:
  klv_1202_local_set_format();

  std::string
  description_() const override;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1202_key();

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_1202_traits_lookup();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
