// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1010 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1010_H_
#define KWIVER_ARROWS_KLV_KLV_1010_H_

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/optional.h>

#include <bitset>
#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
struct klv_1010_sdcc_flp
{
  // Actual data
  std::vector< klv_lds_key > members;
  std::vector< double > sigma;
  std::vector< double > rho;

  // Directives concerning how that data is to be written
  std::shared_ptr< klv_data_format > sigma_format;
  std::shared_ptr< klv_data_format > rho_format;
  bool long_parse_control;
  bool sparse;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1010_sdcc_flp const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_1010_sdcc_flp )

// ----------------------------------------------------------------------------
/// Interprets data as a ST1010 SDCC-FLP.
class KWIVER_ALGO_KLV_EXPORT klv_1010_sdcc_flp_format
  : public klv_data_format_< klv_1010_sdcc_flp >
{
public:
  klv_1010_sdcc_flp_format();

  klv_1010_sdcc_flp_format( double sigma_minimum, double sigma_maximum );

  std::string
  description() const override;

private:
  klv_1010_sdcc_flp
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1010_sdcc_flp const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_1010_sdcc_flp const& value ) const override;

  bool m_sigma_uses_imap;
  double m_sigma_minimum;
  double m_sigma_maximum;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
