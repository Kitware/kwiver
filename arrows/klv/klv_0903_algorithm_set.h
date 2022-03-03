// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 algorithm local set parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_ALGORITHM_SET_H
#define KWIVER_ARROWS_KLV_KLV_0903_ALGORITHM_SET_H

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_series.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_algorithm_set_tag : klv_lds_key
{
  KLV_0903_ALGORITHM_UNKNOWN    = 0,
  KLV_0903_ALGORITHM_ID         = 1,
  KLV_0903_ALGORITHM_NAME       = 2,
  KLV_0903_ALGORITHM_VERSION    = 3,
  KLV_0903_ALGORITHM_CLASS      = 4,
  KLV_0903_ALGORITHM_NUM_FRAMES = 5,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_algorithm_set_tag tag );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_algorithm_set_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 algorithm local set.
class KWIVER_ALGO_KLV_EXPORT klv_0903_algorithm_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_algorithm_local_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 algorithm series.
using klv_0903_algorithm_series_format =
  klv_series_format< klv_0903_algorithm_local_set_format >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
