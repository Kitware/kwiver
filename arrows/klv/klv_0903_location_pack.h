// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 location pack parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_LOCATION_PACK_H
#define KWIVER_ARROWS_KLV_KLV_0903_LOCATION_PACK_H

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_series.h>
#include <arrows/klv/klv_set.h>

#include <optional>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Standard deviation values along each geospatial axis.
struct KWIVER_ALGO_KLV_EXPORT klv_0903_sigma_pack
{
  double east;
  double north;
  double up;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_sigma_pack const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_0903_sigma_pack )

// ----------------------------------------------------------------------------
/// Correlation values for each pair of geospatial axes.
struct KWIVER_ALGO_KLV_EXPORT klv_0903_rho_pack
{
  double east_north;
  double east_up;
  double north_up;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_rho_pack const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_0903_rho_pack )

// ----------------------------------------------------------------------------
/// Geodetic location with optional precision information.
struct KWIVER_ALGO_KLV_EXPORT klv_0903_location_pack
{
  double latitude;
  double longitude;
  double altitude;
  std::optional< klv_0903_sigma_pack > sigma;
  std::optional< klv_0903_rho_pack > rho;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_location_pack const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_0903_location_pack )

// ----------------------------------------------------------------------------
/// Velocity along geospatial axes with optional precision information.
struct KWIVER_ALGO_KLV_EXPORT klv_0903_velocity_pack
{
  double east;
  double north;
  double up;
  std::optional< klv_0903_sigma_pack > sigma;
  std::optional< klv_0903_rho_pack > rho;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_velocity_pack const& value );

DECLARE_CMP( klv_0903_velocity_pack )

// ----------------------------------------------------------------------------
/// Acceleration along geospatial axes with optional precision information.
using klv_0903_acceleration_pack = klv_0903_velocity_pack;

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 location pack.
class KWIVER_ALGO_KLV_EXPORT klv_0903_location_pack_format
  : public klv_data_format_< klv_0903_location_pack >
{
public:
  klv_0903_location_pack_format();

  std::string
  description_() const override;

private:
  klv_0903_location_pack
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0903_location_pack const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0903_location_pack const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 location series.
using klv_0903_location_series_format =
  klv_series_format< klv_0903_location_pack_format >;

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 velocity pack.
class KWIVER_ALGO_KLV_EXPORT klv_0903_velocity_pack_format
  : public klv_data_format_< klv_0903_velocity_pack >
{
public:
  klv_0903_velocity_pack_format();

  std::string
  description_() const override;

private:
  klv_0903_velocity_pack
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0903_velocity_pack const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0903_velocity_pack const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 acceleration pack.
using klv_0903_acceleration_pack_format = klv_0903_velocity_pack_format;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
