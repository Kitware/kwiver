// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Declaration of KLV demuxer.

#include "klv_timeline.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Holds state for the process of assembling a \c klv_timeline from a
/// sequence of \c klv_packet.
class KWIVER_ALGO_KLV_EXPORT klv_demuxer
{
public:
  using interval_t = typename klv_timeline::interval_t;
  using interval_map_t = typename klv_timeline::interval_map_t;

  /// \param timeline KLV timeline to modify.
  explicit klv_demuxer( klv_timeline& timeline );

  /// Incorporate \p packets into the timeline.
  void send_frame(
    std::vector< klv_packet > const& packets,
    kwiver::vital::optional< uint64_t > backup_timestamp =
      kwiver::vital::nullopt );

  /// Return the timestamp of the most recent frame.
  uint64_t frame_time() const;

  /// Return the timeline being modified.
  klv_timeline&
  timeline() const;

  /// Reset the object to a state equivalent to if it had just been constructed.
  void
  reset();

private:
  using key_t = typename klv_timeline::key_t;

  void demux_packet( klv_packet const& packet );

  void demux_unknown( klv_packet const& packet, uint64_t timestamp );

  void demux_set(
    klv_top_level_tag standard, klv_local_set const& value,
    interval_t const& time_interval, klv_lds_key timestamp_tag = 0 );

  void demux_set(
    klv_top_level_tag standard, klv_universal_set const& value,
    interval_t const& time_interval, klv_lds_key timestamp_tag = 0 );

  void demux_0601( klv_local_set const& value, uint64_t timestamp );

  void demux_1108( klv_local_set const& value, uint64_t timestamp );

  void demux_single_entry( klv_top_level_tag standard,
                           klv_lds_key tag,
                           klv_value const& index,
                           interval_t const& time_interval,
                           klv_value const& value );

  template< class T >
  void demux_list( klv_top_level_tag standard,
                   klv_lds_key tag,
                   interval_t const& time_interval,
                   std::vector< T > const& value );

  uint64_t m_frame_timestamp;
  uint64_t m_prev_frame_timestamp;
  std::multimap< klv_timeline::key_t, uint64_t > m_cancel_points;
  klv_timeline& m_timeline;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver
