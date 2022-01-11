// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Declaration of KLV muxer.

#include "klv_timeline.h"

#include <deque>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Holds state for the process of encoding a \c klv_timeline into sequence of
/// \c klv_packet.
///
/// This class splits the packet-creating process into two steps: sending and
/// receiving frames. This is desirable because there are some situations where
/// it useful for the algorithm to 'read ahead' a number of frames in order to
/// produce an optimally compact encoding. The user should therefore try to
/// maximize the number of frames sent before beginning to request frames back,
/// rather than simply alternating calls to send and receive.
class KWIVER_ALGO_KLV_EXPORT klv_muxer
{
public:
  using interval_t = typename klv_timeline::interval_t;
  using interval_map_t = typename klv_timeline::interval_map_t;

  /// \param timeline KLV timeline to convert.
  explicit klv_muxer( klv_timeline const& timeline );

  /// Read and cache the data between the last frame and the new one at \p
  /// timestamp.
  ///
  /// All information in that timeframe must be present when this function is
  /// called, and can be deleted immediately after.
  void send_frame( uint64_t timestamp );

  /// Return the timestamp of the next cached frame.
  uint64_t next_frame_time() const;

  /// Return the packets associated with the next cached frame.
  std::vector< klv_packet > receive_frame();

  /// Return the timeline being read from.
  klv_timeline const& timeline() const;

private:
  using key_t = typename klv_timeline::key_t;

  void flush_frame();

  void send_frame_unknown( uint64_t timestamp );
  void flush_frame_unknown();

  void send_frame_0104( uint64_t timestamp );
  void flush_frame_0104();

  void send_frame_0601( uint64_t timestamp );
  void flush_frame_0601();

  void send_frame_1108( uint64_t timestamp );
  void flush_frame_1108();

  bool check_timestamp( uint64_t timestamp ) const;

  klv_timeline const& m_timeline;
  std::multimap< uint64_t, klv_packet > m_packets;
  std::deque< uint64_t > m_frames;
  uint64_t m_prev_frame;

  using local_set_cmp_fn =
    bool ( * )( klv_local_set const&, klv_local_set const& );

  std::multiset< klv_local_set, local_set_cmp_fn > m_cached_1108;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver
