// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_H_
#define KWIVER_ARROWS_KLV_KLV_0903_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_tag : klv_lds_key
{
  KLV_0903_UNKNOWN              = 0,
  KLV_0903_CHECKSUM             = 1,
  KLV_0903_PRECISION_TIMESTAMP  = 2,
  KLV_0903_VMTI_SYSTEM_NAME     = 3,
  KLV_0903_VERSION              = 4,
  KLV_0903_NUM_TARGETS_DETECTED = 5,
  KLV_0903_NUM_TARGETS_REPORTED = 6,
  KLV_0903_FRAME_NUMBER         = 7,
  KLV_0903_FRAME_WIDTH          = 8,
  KLV_0903_FRAME_HEIGHT         = 9,
  KLV_0903_SOURCE_SENSOR        = 10,
  KLV_0903_HORIZONTAL_FOV       = 11,
  KLV_0903_VERTICAL_FOV         = 12,
  KLV_0903_MIIS_ID              = 13,

  // Note the jump in tag number here
  KLV_0903_VTARGET_SERIES      = 101,
  KLV_0903_ALGORITHM_SERIES    = 102,
  KLV_0903_ONTOLOGY_SERIES     = 103,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_tag tag );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_0903_key();

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 VMTI local set.
class KWIVER_ALGO_KLV_EXPORT klv_0903_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_local_set_format();

  std::string
  description() const override;

private:
  uint32_t
  calculate_checksum( klv_read_iter_t data, size_t length ) const override;

  uint32_t
  read_checksum( klv_read_iter_t data, size_t length ) const override;

  void
  write_checksum( uint32_t checksum,
                  klv_write_iter_t& data, size_t max_length ) const override;

  size_t
  checksum_length() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
