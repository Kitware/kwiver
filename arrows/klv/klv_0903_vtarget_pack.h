// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 VTarget pack parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_VTARGET_PACK_H_
#define KWIVER_ARROWS_KLV_KLV_0903_VTARGET_PACK_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_series.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_vtarget_pack_tag : klv_lds_key
{
  KLV_0903_VTARGET_UNKNOWN                                = 0,
  KLV_0903_VTARGET_CENTROID                               = 1,
  KLV_0903_VTARGET_BOUNDARY_TOP_LEFT                      = 2,
  KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT                  = 3,
  KLV_0903_VTARGET_PRIORITY                               = 4,
  KLV_0903_VTARGET_CONFIDENCE_LEVEL                       = 5,
  KLV_0903_VTARGET_HISTORY                                = 6,
  KLV_0903_VTARGET_PERCENT_PIXELS                         = 7,
  KLV_0903_VTARGET_COLOR                                  = 8,
  KLV_0903_VTARGET_INTENSITY                              = 9,
  KLV_0903_VTARGET_LOCATION_OFFSET_LATITUDE               = 10,
  KLV_0903_VTARGET_LOCATION_OFFSET_LONGITUDE              = 11,
  KLV_0903_VTARGET_LOCATION_ELLIPSOID_HEIGHT              = 12,
  KLV_0903_VTARGET_BOUNDARY_TOP_LEFT_LATITUDE_OFFSET      = 13,
  KLV_0903_VTARGET_BOUNDARY_TOP_LEFT_LONGITUDE_OFFSET     = 14,
  KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT_LATITUDE_OFFSET  = 15,
  KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT_LONGITUDE_OFFSET = 16,
  KLV_0903_VTARGET_LOCATION                               = 17,
  KLV_0903_VTARGET_BOUNDARY_SERIES                        = 18,
  KLV_0903_VTARGET_CENTROID_ROW                           = 19,
  KLV_0903_VTARGET_CENTROID_COLUMN                        = 20,
  KLV_0903_VTARGET_FPA_INDEX                              = 21,
  KLV_0903_VTARGET_ALGORITHM_ID                           = 22,

  // Note the jump in tag number here
  KLV_0903_VTARGET_VMASK                                  = 101,
  KLV_0903_VTARGET_VOBJECT                                = 102,
  KLV_0903_VTARGET_VFEATURE                               = 103,
  KLV_0903_VTARGET_VTRACKER                               = 104,
  KLV_0903_VTARGET_VCHIP                                  = 105,
  KLV_0903_VTARGET_VCHIP_SERIES                           = 106,
  KLV_0903_VTARGET_VOBJECT_SERIES                         = 107,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vtarget_pack_tag tag );

// ----------------------------------------------------------------------------
/// Two-dimensional index into the Focal Plane Array.
struct KWIVER_ALGO_KLV_EXPORT klv_0903_fpa_index
{
  uint8_t row;
  uint8_t column;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_fpa_index const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_0903_fpa_index )

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 Focal Plane Array index pack.
class KWIVER_ALGO_KLV_EXPORT klv_0903_fpa_index_format
  : public klv_data_format_< klv_0903_fpa_index >
{
public:
  klv_0903_fpa_index_format();

  std::string
  description() const override;

private:
  klv_0903_fpa_index
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0903_fpa_index const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0903_fpa_index const& value ) const override;
};

// ----------------------------------------------------------------------------
/// An integer id paired with a ST0903 vTarget local set.
struct KWIVER_ALGO_KLV_EXPORT klv_0903_vtarget_pack
{
  uint64_t id;
  klv_value set;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vtarget_pack const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_0903_vtarget_pack )

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vTarget pack.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vtarget_pack_format
  : public klv_data_format_< klv_0903_vtarget_pack >
{
public:
  klv_0903_vtarget_pack_format();

  std::string
  description() const override;

private:
  klv_0903_vtarget_pack
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0903_vtarget_pack const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0903_vtarget_pack const& value ) const override;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_vtarget_pack_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vTarget local set.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vtarget_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_vtarget_local_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vTarget series.
using klv_0903_vtarget_series_format =
  klv_series_format< klv_0903_vtarget_pack_format >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
