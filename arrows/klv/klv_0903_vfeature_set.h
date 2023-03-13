// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 VFeature local set parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_VFEATURE_SET_H
#define KWIVER_ARROWS_KLV_KLV_0903_VFEATURE_SET_H

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_vfeature_set_tag : klv_lds_key
{
  KLV_0903_VFEATURE_UNKNOWN        = 0,
  KLV_0903_VFEATURE_SCHEMA         = 1,
  KLV_0903_VFEATURE_SCHEMA_FEATURE = 2,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vfeature_set_tag tag );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_vfeature_set_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vFeature local set.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vfeature_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_vfeature_local_set_format();

  std::string
  description_() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
