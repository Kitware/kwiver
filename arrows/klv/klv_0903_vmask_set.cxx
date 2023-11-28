// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 VMask local set parser.

#include "klv_0903_vmask_set.h"

#include <arrows/klv/klv_series.hpp>
#include <arrows/klv/klv_util.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vmask_set_tag tag )
{
  return os << klv_0903_vmask_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_pixel_run const& value )
{
  os    << "{ "
        << "index: " << value.index << ", "
        << "length: " << value.length
        << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0903_pixel_run,
  &klv_0903_pixel_run::index,
  &klv_0903_pixel_run::length
  )

// ----------------------------------------------------------------------------
klv_0903_pixel_run_format
::klv_0903_pixel_run_format()
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_pixel_run_format
::description_() const
{
  return "ST0903 Pixel Run Pack";
}

// ----------------------------------------------------------------------------
klv_0903_pixel_run
klv_0903_pixel_run_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_0903_pixel_run result;
  auto const length_of_index =
    klv_read_ber< size_t >( data, tracker.remaining() );
  result.index =
    klv_read_int< uint64_t >( data, tracker.verify( length_of_index ) );
  result.length = klv_read_ber< uint64_t >( data, tracker.remaining() );
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0903_pixel_run_format
::write_typed( klv_0903_pixel_run const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  auto const length_of_index = klv_int_length( value.index );
  klv_write_ber( length_of_index, data, tracker.remaining() );
  klv_write_int( value.index, data, tracker.verify( length_of_index ) );
  klv_write_ber( value.length, data, tracker.remaining() );
}

// ----------------------------------------------------------------------------
size_t
klv_0903_pixel_run_format
::length_of_typed( klv_0903_pixel_run const& value ) const
{
  auto const length_of_index = klv_int_length( value.index );
  auto const length_of_length_of_index = klv_ber_length( length_of_index );
  auto const length_of_length = klv_ber_length( value.length );
  return length_of_index + length_of_length_of_index + length_of_length;
}

// ----------------------------------------------------------------------------
klv_0903_vmask_local_set_format
::klv_0903_vmask_local_set_format()
  : klv_local_set_format{ klv_0903_vmask_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_vmask_local_set_format
::description_() const
{
  return "ST0903 VMask LS";
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_vmask_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_VMASK_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VMASK_PIXEL_CONTOUR ),
      std::make_shared< klv_uint_series_format >(),
      "Pixel Contour",
      "Series of points listed in clockwise order. Each point is represented "
      "by an integer indicating the pixel index. Uses the equation "
      "(row - 1) * width + column, where row and column are 1-indexed.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VMASK_BITMASK_SERIES ),
      std::make_shared< klv_0903_pixel_run_series_format >(),
      "Bitmask Series",
      "Bitmask describing the pixels that subtend the target within the "
      "frame.",
      { 0, 1 } }, };

  return lookup;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
