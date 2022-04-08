// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 VMask local set parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_VMASK_SET_H_
#define KWIVER_ARROWS_KLV_KLV_0903_VMASK_SET_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_series.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_vmask_set_tag : klv_lds_key
{
  KLV_0903_VMASK_UNKNOWN        = 0,
  KLV_0903_VMASK_POLYGON        = 1,
  KLV_0903_VMASK_BITMASK_SERIES = 2,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vmask_set_tag tag );

// ----------------------------------------------------------------------------
/// Specifies a continuous left-to-right span of pixels.
struct KWIVER_ALGO_KLV_EXPORT klv_0903_pixel_run
{
  uint64_t index;
  uint64_t length;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_pixel_run const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_0903_pixel_run )

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 pixel run.
class KWIVER_ALGO_KLV_EXPORT klv_0903_pixel_run_format
  : public klv_data_format_< klv_0903_pixel_run >
{
public:
  klv_0903_pixel_run_format();

  std::string
  description() const override;

private:
  klv_0903_pixel_run
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0903_pixel_run const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0903_pixel_run const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 pixel run series.
using klv_0903_pixel_run_series_format =
  klv_series_format< klv_0903_pixel_run_format >;

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vMask local set.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vmask_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_vmask_local_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_vmask_set_traits_lookup();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
