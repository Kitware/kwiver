// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV list format.

#ifndef KWIVER_ARROWS_KLV_KLV_LIST_H_
#define KWIVER_ARROWS_KLV_KLV_LIST_H_

#include <arrows/klv/klv_data_format.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Interprets data as a list of unknown cardinality.
template < class Format >
class KWIVER_ALGO_KLV_EXPORT klv_list_format
  : public klv_data_format_< std::vector< typename Format::data_type > >
{
public:
  template < class... Args >
  klv_list_format( Args&&... args );

  std::string
  description() const override;

private:
  using element_t = typename Format::data_type;

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

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
