// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV series format.

#ifndef KWIVER_ARROWS_KLV_KLV_SERIES_H_
#define KWIVER_ARROWS_KLV_KLV_SERIES_H_

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <ostream>
#include <vector>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class Format >
class KWIVER_ALGO_KLV_EXPORT klv_series_format
  : public klv_data_format_< std::vector< typename Format::data_type > >
{
public:
  using element_t = typename Format::data_type;

  template < class... Args >
  klv_series_format( Args&&... args );

  std::string
  description() const override;

protected:
  std::vector< element_t >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( std::vector< element_t > const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( std::vector< element_t > const& value ) const override;

  std::ostream&
  print_typed( std::ostream& os,
               std::vector< element_t > const& value ) const override;

  Format m_format;
};

// ----------------------------------------------------------------------------
using klv_uint_series_format = klv_series_format< klv_uint_format >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
