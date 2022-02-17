// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 VTrackItem local set parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_VTRACKITEM_SET_H
#define KWIVER_ARROWS_KLV_KLV_0903_VTRACKITEM_SET_H

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_series.h>
#include <arrows/klv/klv_set.h>
#include <arrows/klv/klv_util.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_vtrackitem_pack_tag : klv_lds_key
{
  KLV_0903_VTRACKITEM_UNKNOWN               = 0,
  KLV_0903_VTRACKITEM_TIMESTAMP             = 1,
  KLV_0903_VTRACKITEM_CENTROID              = 2,
  KLV_0903_VTRACKITEM_CENTROID_ROW          = 3,
  KLV_0903_VTRACKITEM_CENTROID_COLUMN       = 4,
  KLV_0903_VTRACKITEM_BOUNDARY_TOP_LEFT     = 5,
  KLV_0903_VTRACKITEM_BOUNDARY_BOTTOM_RIGHT = 6,
  KLV_0903_VTRACKITEM_PRIORITY              = 7,
  KLV_0903_VTRACKITEM_CONFIDENCE_LEVEL      = 8,
  KLV_0903_VTRACKITEM_HISTORY               = 9,
  KLV_0903_VTRACKITEM_PERCENT_PIXELS        = 10,
  KLV_0903_VTRACKITEM_COLOR                 = 11,
  KLV_0903_VTRACKITEM_INTENSITY             = 12,
  KLV_0903_VTRACKITEM_LOCATION              = 13,
  KLV_0903_VTRACKITEM_BOUNDARY_SERIES       = 14,
  KLV_0903_VTRACKITEM_VELOCITY              = 15,
  KLV_0903_VTRACKITEM_ACCELERATION          = 16,
  KLV_0903_VTRACKITEM_FPA_INDEX             = 17,
  KLV_0903_VTRACKITEM_FRAME_NUMBER          = 18,
  KLV_0903_VTRACKITEM_MIIS_ID               = 19,
  KLV_0903_VTRACKITEM_FRAME_WIDTH           = 20,
  KLV_0903_VTRACKITEM_FRAME_HEIGHT          = 21,
  KLV_0903_VTRACKITEM_HORIZONTAL_FOV        = 22,
  KLV_0903_VTRACKITEM_VERTICAL_FOV          = 23,
  KLV_0903_VTRACKITEM_MI_URL                = 24,

  // Note the jump in tag number here
  KLV_0903_VTRACKITEM_VMASK                 = 101,
  KLV_0903_VTRACKITEM_VOBJECT               = 102,
  KLV_0903_VTRACKITEM_VFEATURE              = 103,
  KLV_0903_VTRACKITEM_VCHIP                 = 104,
  KLV_0903_VTRACKITEM_VCHIP_SERIES          = 105,
  KLV_0903_VTRACKITEM_VOBJECT_SERIES        = 106,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vtrackitem_pack_tag tag );

// ----------------------------------------------------------------------------
/// An integer id paired with a ST0903 vTrackItem local set.
struct KWIVER_ALGO_KLV_EXPORT klv_0903_vtrackitem_pack
{
  uint64_t id;
  klv_local_set set;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vtrackitem_pack const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_0903_vtrackitem_pack )

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vTrackItem pack.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vtrackitem_pack_format
  : public klv_data_format_< klv_0903_vtrackitem_pack >
{
public:
  klv_0903_vtrackitem_pack_format();

  std::string
  description() const override;

private:
  klv_0903_vtrackitem_pack
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0903_vtrackitem_pack const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0903_vtrackitem_pack const& value ) const override;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_vtrackitem_pack_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vTrackItem local set.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vtrackitem_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_vtrackitem_local_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
/// Series of ST0903 vTrackItem local sets.
using klv_0903_vtrackitem_series =
  klv_series< klv_0903_vtrackitem_local_set_format >;

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vTrackItem series.
using klv_0903_vtrackitem_series_format =
  klv_series_format< klv_0903_vtrackitem_local_set_format >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
