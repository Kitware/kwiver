// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of KLV update interval settings.

#ifndef KWIVER_ARROWS_KLV_KLV_UPDATE_INTERVALS_H_
#define KWIVER_ARROWS_KLV_KLV_UPDATE_INTERVALS_H_

#include <arrows/klv/klv_timeline.h>

#include <vital/util/variant/variant.hpp>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Names for recommended update intervals.
enum : uint64_t
{
  // Update every frame.
  // Recommended for quickly changing values.
  KLV_UPDATE_INTERVAL_MIN     = 0,

  // Recommended for generally unchanging values.
  KLV_UPDATE_INTERVAL_LONG    = 10000000,

  // Maximum allowed update interval.
  // Technically should be 30 seconds, but we don't want one or two missed
  // frames to result in a timeout, so we leave one second of buffer time.
  KLV_UPDATE_INTERVAL_MAX     = 29000000,

  // Default update interval when not otherwise specified.
  KLV_UPDATE_INTERVAL_DEFAULT = KLV_UPDATE_INTERVAL_MIN,
};

// ----------------------------------------------------------------------------
/// Specification of how often unchanging KLV values should repeat.
///
/// More localized specifications override more general ones. All intervals
/// measured in microseconds.
class KWIVER_ALGO_KLV_EXPORT klv_update_intervals
{
public:
  struct KWIVER_ALGO_KLV_EXPORT key_t
  {
    key_t( klv_top_level_tag standard );
    key_t( klv_top_level_tag standard, std::optional< klv_lds_key > tag );

    klv_top_level_tag standard;
    std::optional< klv_lds_key > tag;
  };

  using value_t = uint64_t;
  using container_t = std::map< key_t, uint64_t >;

  klv_update_intervals();
  klv_update_intervals(
    std::initializer_list< container_t::value_type > const& items );

  /// Return the update interval at \p key.
  value_t at( key_t const& key ) const;

  /// Set the default update interval for all standards to \p value.
  void set( value_t value );

  /// Set the update interval for \p key to \p value.
  void set( key_t const& key, value_t value );

private:
  value_t m_default;
  container_t m_map;
};

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_update_intervals::key_t );

// ----------------------------------------------------------------------------
/// Return reasonable, MISB-compliant update intervals for all supported
/// standards.
KWIVER_ALGO_KLV_EXPORT
klv_update_intervals const&
klv_recommended_update_intervals();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
