// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1601 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1601_H_
#define KWIVER_ARROWS_KLV_KLV_1601_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_1303.h>
#include <arrows/klv/klv_imap.h>
#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1601_tag : klv_lds_key
{
  KLV_1601_UNKNOWN             = 0,
  KLV_1601_VERSION             = 1,
  KLV_1601_ALGORITHM_NAME      = 2,
  KLV_1601_ALGORITHM_VERSION   = 3,
  KLV_1601_PIXEL_POINTS        = 4,
  KLV_1601_GEOGRAPHIC_POINTS   = 5,
  KLV_1601_SECOND_IMAGE_NAME   = 6,
  KLV_1601_ALGORITHM_CONFIG_ID = 7,
  KLV_1601_ELEVATION           = 8,
  KLV_1601_PIXEL_SDCC          = 9,
  KLV_1601_GEOGRAPHIC_SDCC     = 10,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1601_tag tag );

// ----------------------------------------------------------------------------
/// Interprets data as a ST1601 pixel SDCC MDARRAY.
///
/// This is different from a standard MDARRAY, since different rows use
/// different IMAP parameters, which is not supported natively by the MDARRAY
/// format.
class KWIVER_ALGO_KLV_EXPORT klv_1601_pixel_sdcc_format
  : public klv_data_format_< klv_1303_mdap< klv_imap > >
{
public:
  klv_1601_pixel_sdcc_format();

  std::string
  description_() const override;

private:
  klv_1303_mdap< klv_imap >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1303_mdap< klv_imap > const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_1303_mdap< klv_imap > const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST1601 geographic SDCC MDARRAY.
///
/// This is different from a standard MDARRAY, since different rows use
/// different IMAP parameters, which is not supported natively by the MDARRAY
/// format.
class KWIVER_ALGO_KLV_EXPORT klv_1601_geographic_sdcc_format
  : public klv_data_format_< klv_1303_mdap< klv_imap > >
{
public:
  klv_1601_geographic_sdcc_format();

  std::string
  description_() const override;

private:
  klv_1303_mdap< klv_imap >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1303_mdap< klv_imap > const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_1303_mdap< klv_imap > const& value ) const override;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1601_key();

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_1601_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST1601 geo-registration local set.
class KWIVER_ALGO_KLV_EXPORT klv_1601_local_set_format
  : public klv_local_set_format
{
public:
  klv_1601_local_set_format();

  std::string
  description_() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
