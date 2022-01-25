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

#include <initializer_list>
#include <ostream>
#include <vector>

namespace kwiver {

namespace arrows {

namespace klv {

template < class Format >
class KWIVER_ALGO_KLV_EXPORT klv_series
{
public:
  using data_type = typename Format::data_type;
  using container_t = std::vector< klv_value >;

  template < class... Args >
  klv_series( container_t const& elements, Args&&... args );

  container_t& operator*();

  container_t const& operator*() const;

  container_t* operator->();

  container_t const* operator->() const;

  Format const&
  format() const;

private:
  Format m_format;
  container_t m_elements;
};

// ----------------------------------------------------------------------------
template < class Format >
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_series< Format > const& value );

// ----------------------------------------------------------------------------
template < class Format >
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_series< Format > const& lhs, klv_series< Format > const& rhs );

// ----------------------------------------------------------------------------
template < class Format >
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_series< Format > const& lhs, klv_series< Format > const& rhs );

// ----------------------------------------------------------------------------
template < class Format >
class KWIVER_ALGO_KLV_EXPORT klv_series_format
  : public klv_data_format_< klv_series< Format > >
{
public:
  using klv_series_t = klv_series< Format >;

  template < class... Args >
  klv_series_format( Args&&... args );

  std::string
  description() const override;

protected:
  klv_series_t
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_series_t const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_series_t const& value,
                   size_t length_hint ) const override;

  Format m_format;
};

// ----------------------------------------------------------------------------
using klv_uint_series = klv_series< klv_uint_format >;

// ----------------------------------------------------------------------------
using klv_uint_series_format = klv_series_format< klv_uint_format >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
