// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1303 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1303_H_
#define KWIVER_ARROWS_KLV_KLV_1303_H_

#include <arrows/klv/klv_set.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/util/interval.h>

#include <array>
#include <optional>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Indicates method of encoding used.
enum klv_1303_apa
{
  KLV_1303_APA_UNKNOWN = 0,
  KLV_1303_APA_NATURAL = 1,
  KLV_1303_APA_IMAP    = 2,
  KLV_1303_APA_BOOLEAN = 3,
  KLV_1303_APA_UINT    = 4,
  KLV_1303_APA_RLE     = 5,
  KLV_1303_APA_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1303_apa value );

// ----------------------------------------------------------------------------
using klv_1303_apa_format = klv_enum_format< klv_1303_apa >;

// ----------------------------------------------------------------------------
/// KLV ST1303 multi-dimensional array.
template < class T >
struct KWIVER_ALGO_KLV_EXPORT klv_1303_mdap
{
  using value_type = T;

  // Actual data
  std::vector< size_t > sizes;
  std::vector< T > elements;

  // Parameters determining how to encode that data
  size_t element_size;
  klv_1303_apa apa;
  size_t apa_params_length;
  std::optional< kwiver::vital::interval< double > > imap_params;
};

// ----------------------------------------------------------------------------
template < class T >
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1303_mdap< T > const& value );

// ----------------------------------------------------------------------------
DECLARE_TEMPLATE_CMP( klv_1303_mdap< T > )

// ----------------------------------------------------------------------------
/// Interprets data as a ST1303 MDAP/MDARRAY.
template < class Format >
class klv_1303_mdap_format
  : public klv_data_format_< klv_1303_mdap< typename Format::data_type > >
{
public:
  template < class... Args >
  klv_1303_mdap_format( Args&&... args );

  std::string
  description() const override;

private:
  using element_t = typename Format::data_type;
  using mdap_t = klv_1303_mdap< element_t >;

  mdap_t
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( mdap_t const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( mdap_t const& value ) const override;

  Format m_format;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
