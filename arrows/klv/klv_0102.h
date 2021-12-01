// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface to the KLV 0102 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0102_H_
#define KWIVER_ARROWS_KLV_KLV_0102_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include "klv_packet.h"
#include "klv_set.h"

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0102_tag : klv_lds_key
{
  KLV_0102_UNKNOWN                                   = 0,
  KLV_0102_SECURITY_CLASSIFICATION                   = 1,
  KLV_0102_COUNTRY_CODING_METHOD                     = 2,
  KLV_0102_CLASSIFYING_COUNTRY                       = 3,
  KLV_0102_SCI_SHI_INFORMATION                       = 4,
  KLV_0102_CAVEATS                                   = 5,
  KLV_0102_RELEASING_INSTRUCTIONS                    = 6,
  KLV_0102_CLASSIFIED_BY                             = 7,
  KLV_0102_DERIVED_FROM                              = 8,
  KLV_0102_CLASSIFICATION_REASON                     = 9,
  KLV_0102_DECLASSIFICATION_DATE                     = 10,
  KLV_0102_CLASSIFICATION_AND_MARKING_SYSTEM         = 11,
  KLV_0102_OBJECT_COUNTRY_CODING_METHOD              = 12,
  KLV_0102_OBJECT_COUNTRY_CODES                      = 13,
  KLV_0102_CLASSIFICATION_COMMENTS                   = 14,
  KLV_0102_VERSION                                   = 15,
  KLV_0102_COUNTRY_CODING_METHOD_VERSION_DATE        = 16,
  KLV_0102_OBJECT_COUNTRY_CODING_METHOD_VERSION_DATE = 17,
  KLV_0102_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0102_tag tag );

// ----------------------------------------------------------------------------
/// Indicates the security classification level of the KLV metadata.
enum KWIVER_ALGO_KLV_EXPORT klv_0102_security_classification
{
  KLV_0102_SECURITY_CLASSIFICATION_UNKNOWN      = 0x00,
  KLV_0102_SECURITY_CLASSIFICATION_UNCLASSIFIED = 0x01,
  KLV_0102_SECURITY_CLASSIFICATION_RESTRICTED   = 0x02,
  KLV_0102_SECURITY_CLASSIFICATION_CONFIDENTIAL = 0x03,
  KLV_0102_SECURITY_CLASSIFICATION_SECRET       = 0x04,
  KLV_0102_SECURITY_CLASSIFICATION_TOP_SECRET   = 0x05,
  KLV_0102_SECURITY_CLASSIFICATION_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0102_security_classification value );

// ----------------------------------------------------------------------------
using klv_0102_security_classification_format =
  klv_enum_format< klv_0102_security_classification >;

// ----------------------------------------------------------------------------
/// Indicates the system by which a string maps to a country.
enum KWIVER_ALGO_KLV_EXPORT klv_0102_country_coding_method
{
  KLV_0102_COUNTRY_CODING_METHOD_UNKNOWN               = 0x00,
  KLV_0102_COUNTRY_CODING_METHOD_ISO_3166_TWO_LETTER   = 0x01,
  KLV_0102_COUNTRY_CODING_METHOD_ISO_3166_THREE_LETTER = 0x02,
  KLV_0102_COUNTRY_CODING_METHOD_FIPS_10_4_TWO_LETTER  = 0x03,
  KLV_0102_COUNTRY_CODING_METHOD_FIPS_10_4_FOUR_LETTER = 0x04,
  KLV_0102_COUNTRY_CODING_METHOD_ISO_3166_NUMERIC      = 0x05,
  KLV_0102_COUNTRY_CODING_METHOD_1059_TWO_LETTER       = 0x06,
  KLV_0102_COUNTRY_CODING_METHOD_1059_THREE_LETTER     = 0x07,
  KLV_0102_COUNTRY_CODING_METHOD_FIPS_10_4_MIXED       = 0x0A,
  KLV_0102_COUNTRY_CODING_METHOD_ISO_3166_MIXED        = 0x0B,
  KLV_0102_COUNTRY_CODING_METHOD_STANAG_1059_MIXED     = 0x0C,
  KLV_0102_COUNTRY_CODING_METHOD_GENC_TWO_LETTER       = 0x0D,
  KLV_0102_COUNTRY_CODING_METHOD_GENC_THREE_LETTER     = 0x0E,
  KLV_0102_COUNTRY_CODING_METHOD_GENC_NUMERIC          = 0x0F,
  KLV_0102_COUNTRY_CODING_METHOD_GENC_MIXED            = 0x10,
  KLV_0102_COUNTRY_CODING_METHOD_GENC_ADMIN_SUB        = 0x40,
  KLV_0102_COUNTRY_CODING_METHOD_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0102_country_coding_method value );

// ----------------------------------------------------------------------------
using klv_0102_country_coding_method_format =
  klv_enum_format< klv_0102_country_coding_method >;

// ----------------------------------------------------------------------------
/// Interprets data as an ST0102 local set.
class KWIVER_ALGO_KLV_EXPORT klv_0102_local_set_format
  : public klv_local_set_format
{
public:
  klv_0102_local_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
/// Returns the UDS key for a MISB ST0102 local set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_0102_key();

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST0102 tags.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0102_traits_lookup();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
