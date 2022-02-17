// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1108 Metric Local Set parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1108_METRIC_SET_H_
#define KWIVER_ARROWS_KLV_KLV_1108_METRIC_SET_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include "klv_key.h"
#include "klv_set.h"
#include "klv_util.h"

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1108_metric_set_tag : klv_lds_key
{
  KLV_1108_METRIC_SET_UNKNOWN     = 0,
  KLV_1108_METRIC_SET_NAME        = 1,
  KLV_1108_METRIC_SET_VERSION     = 2,
  KLV_1108_METRIC_SET_IMPLEMENTER = 3,
  KLV_1108_METRIC_SET_PARAMETERS  = 4,
  KLV_1108_METRIC_SET_TIME        = 5,
  KLV_1108_METRIC_SET_VALUE       = 6,
  KLV_1108_METRIC_SET_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1108_metric_set_tag tag );

// ----------------------------------------------------------------------------
/// Indicates who implemented the software which calculated the metric.
struct KWIVER_ALGO_KLV_EXPORT klv_1108_metric_implementer {
  std::string organization;
  std::string subgroup;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1108_metric_implementer const& rhs );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_1108_metric_implementer )

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1108 metric local set implementer.
class KWIVER_ALGO_KLV_EXPORT klv_1108_metric_implementer_format
  : public klv_data_format_< klv_1108_metric_implementer >
{
public:
  klv_1108_metric_implementer_format();

  std::string
  description() const override;

private:
  klv_1108_metric_implementer
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1108_metric_implementer const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_1108_metric_implementer const& value,
                   size_t length_hint ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1108 metric local set.
class KWIVER_ALGO_KLV_EXPORT klv_1108_metric_local_set_format
  : public klv_local_set_format
{
public:
  klv_1108_metric_local_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1108_metric_set_key();

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_1108_metric_set_traits_lookup();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
