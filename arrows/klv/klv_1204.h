// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1204 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1204_H_
#define KWIVER_ARROWS_KLV_KLV_1204_H_

#include <arrows/klv/kwiver_algo_klv_export.h>
#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_uuid.h>

#include <vital/optional.h>

#include <array>
#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1204_device_id_type : uint8_t
{
  KLV_1204_DEVICE_ID_TYPE_NONE     = 0,
  KLV_1204_DEVICE_ID_TYPE_MANAGED  = 1,
  KLV_1204_DEVICE_ID_TYPE_VIRTUAL  = 2,
  KLV_1204_DEVICE_ID_TYPE_PHYSICAL = 3,
  KLV_1204_DEVICE_ID_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1204_device_id_type value );

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_KLV_EXPORT klv_1204_miis_id
{
  uint8_t version;
  klv_1204_device_id_type sensor_id_type;
  klv_1204_device_id_type platform_id_type;
  kwiver::vital::optional< klv_uuid > sensor_id;
  kwiver::vital::optional< klv_uuid > platform_id;
  kwiver::vital::optional< klv_uuid > window_id;
  kwiver::vital::optional< klv_uuid > minor_id;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1204_miis_id const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_1204_miis_id )

// ----------------------------------------------------------------------------
/// Interprets data as a ST1204 MIIS ID.
class KWIVER_ALGO_KLV_EXPORT klv_1204_miis_id_format
  : public klv_data_format_< klv_1204_miis_id >
{
public:
  klv_1204_miis_id_format();

  std::string
  description() const override;

private:
  klv_1204_miis_id
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1204_miis_id const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_1204_miis_id const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Returns the UDS key for a MISB ST1204 MIIS ID.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1204_key();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
