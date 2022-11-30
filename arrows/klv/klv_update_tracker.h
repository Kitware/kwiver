// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of KLV update tracker.

#ifndef KWIVER_ARROWS_KLV_KLV_UPDATE_TRACKER_H_
#define KWIVER_ARROWS_KLV_KLV_UPDATE_TRACKER_H_

#include <arrows/klv/klv_update_intervals.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Utility class to aid in the pruning of Report-On-Change set entries.
template< class Key >
class KWIVER_ALGO_KLV_EXPORT klv_update_tracker
{
public:
  struct key_t
  {
    klv_top_level_tag standard;
    Key tag;

    bool operator<( key_t const& other ) const;
  };

  struct value_t
  {
    uint64_t timestamp;
    std::set< klv_value > value;

    bool operator<( value_t const& other ) const;
  };

  klv_update_tracker();

  value_t const* at( key_t const& key ) const;

  bool has_changed( klv_set< Key > const& set, key_t const& key ) const;

  bool update( klv_set< Key > const& set, key_t const& key,
               uint64_t timestamp );

  void prune(
    klv_set< Key >& set, klv_update_intervals const& intervals,
    klv_top_level_tag standard, uint64_t timestamp );

private:
  std::map< key_t, value_t > m_map;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
