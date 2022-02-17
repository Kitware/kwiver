// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1108 Metric Local Set parser.

#include "klv_1108_metric_set.h"

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

constexpr auto metric_implementer_separator = '\x1E';

} // namespace <anonymous>

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_metric_set_tag tag )
{
  return os << klv_1108_metric_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_1108_metric_implementer,
  &klv_1108_metric_implementer::organization,
  &klv_1108_metric_implementer::subgroup
)

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_metric_implementer const& rhs )
{
  return os << "{ Organization: \"" << rhs.organization << "\", "
            << "Subgroup: \"" << rhs.subgroup << "\" }";
}

// ----------------------------------------------------------------------------
klv_1108_metric_implementer_format
::klv_1108_metric_implementer_format()
  : klv_data_format_< klv_1108_metric_implementer >{ 0 }
{}

// ----------------------------------------------------------------------------
klv_1108_metric_implementer
klv_1108_metric_implementer_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  // Split by separator character
  auto const s = klv_read_string( data, length );
  auto const pos = s.find( metric_implementer_separator );
  if( pos == s.npos )
  {
    LOG_WARN( kwiver::vital::get_logger( "klv" ),
              "separator character 0x1E not found "
              "in metric implementer string" );
    return { s, "" };
  }
  return { s.substr( 0, pos ), s.substr( pos + 1 ) };
}

// ----------------------------------------------------------------------------
void
klv_1108_metric_implementer_format
::write_typed( klv_1108_metric_implementer const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const s = value.organization + metric_implementer_separator +
                 value.subgroup;
  klv_write_string( s, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_1108_metric_implementer_format
::length_of_typed( klv_1108_metric_implementer const& value,
                   VITAL_UNUSED size_t length_hint ) const
{
  // Add one byte for separator character
  return value.organization.size() + 1 + value.subgroup.size();
}

// ----------------------------------------------------------------------------
std::string
klv_1108_metric_implementer_format
::description() const
{
  return "metric implementer of " + length_description();
}

// ----------------------------------------------------------------------------
klv_1108_metric_local_set_format
::klv_1108_metric_local_set_format()
  : klv_local_set_format{ klv_1108_metric_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_1108_metric_local_set_format
::description() const
{
  std::stringstream ss;
  ss << "ST 1108 metric local set";
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1108_metric_set_key()
{
  // From Table 1 of https://gwg.nga.mil/misb/docs/standards/ST1108.3.pdf
  return { 0x060E2B3402030101, 0x0E01050100000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_1108_metric_set_traits_lookup()
{
#define ENUM_AND_NAME( X ) X, #X
  // Constants here are taken from Table 5 of
  // https://gwg.nga.mil/misb/docs/standards/ST1108.3.pdf
  // Descriptions are edited for clarity, brevity, consistency, etc.
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_1108_METRIC_SET_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Tag",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010101, 0x0E01050700000000 }, // "Key" column
      ENUM_AND_NAME( KLV_1108_METRIC_SET_NAME ),  // KWIVER enumeration
      std::make_shared< klv_string_format >(),    // "Type" column
      "Metric Name",                              // "Item Name" column
      "Examples: 'VNIIRS', 'RER', 'GSD'.",        // "Notes" column
      1 },                                        // "M/O" column (mandatory)
    { { 0x060E2B3401010101, 0x0E01050800000000 },
      ENUM_AND_NAME( KLV_1108_METRIC_SET_VERSION ),
      std::make_shared< klv_string_format >(),
      "Metric Version",
      "Alphanumeric denoting calculated values. 'Human' for observed.",
      1 },
    { { 0x060E2B3401010101, 0x0E01050900000000 },
      ENUM_AND_NAME( KLV_1108_METRIC_SET_IMPLEMENTER ),
      std::make_shared< klv_1108_metric_implementer_format >(),
      "Metric Implementer",
      "Identifies organization responsible for how metric is calculated.",
      1 },
    { { 0x060E2B3401010101, 0x0E01050A00000000 },
      ENUM_AND_NAME( KLV_1108_METRIC_SET_PARAMETERS ),
      std::make_shared< klv_string_format >(),
      "Metric Parameters",
      "Additional information needed to replicate the calculation.",
      { 0, 1 } },
    { { 0x060E2B3401010103, 0x0702010101050000 },
      ENUM_AND_NAME( KLV_1108_METRIC_SET_TIME ),
      std::make_shared< klv_uint_format >( 8 ),
      "Metric Time",
      "Time of metric assessment. MISP Precision Timestamp.",
      1 },
    { { 0x060E2B3401010101, 0x0E01050B00000000 },
      ENUM_AND_NAME( KLV_1108_METRIC_SET_VALUE ),
      std::make_shared< klv_float_format >(),
      "Metric Value",
      "Numeric value of calculation.",
      1 } };
#undef ENUM_AND_NAME
  return lookup;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
