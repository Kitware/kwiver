// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 VObject local set parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_VOBJECT_SET_H_
#define KWIVER_ARROWS_KLV_KLV_0903_VOBJECT_SET_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_series.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_vobject_set_tag : klv_lds_key
{
  KLV_0903_VOBJECT_UNKNOWN        = 0,
  KLV_0903_VOBJECT_ONTOLOGY       = 1,
  KLV_0903_VOBJECT_ONTOLOGY_CLASS = 2,
  KLV_0903_VOBJECT_ONTOLOGY_ID    = 3,
  KLV_0903_VOBJECT_CONFIDENCE     = 4,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vobject_set_tag tag );

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vObject local set.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vobject_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_vobject_local_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_vobject_set_traits_lookup();

// ----------------------------------------------------------------------------
/// Series of ST0903 vObject local sets.
using klv_0903_vobject_series =
  klv_series< klv_0903_vobject_local_set_format >;

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vObject series.
using klv_0903_vobject_series_format =
  klv_series_format< klv_0903_vobject_local_set_format >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
