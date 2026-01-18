// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header file for \link kwiver::vital::algo::query_track_descriptor_set
///        query_track_descriptor_set \endlink

#ifndef VITAL_QUERY_TRACK_DESCRIPTOR_SET_H_
#define VITAL_QUERY_TRACK_DESCRIPTOR_SET_H_

#include <vital/algo/algorithm.h>
#include <vital/vital_export.h>

#include <vital/types/track.h>
#include <vital/types/track_descriptor.h>

namespace kwiver {

namespace vital {

namespace algo {

// ------------------------------------------------------------------
/// Abstract interface for a collection of track descriptors that can be queried
class VITAL_ALGO_EXPORT query_track_descriptor_set
  : public kwiver::vital::algorithm
{
public:
  query_track_descriptor_set();
  PLUGGABLE_INTERFACE( query_track_descriptor_set );

  /// Tuple containing video name, descriptor, and tracks
  typedef std::tuple< std::string,
    vital::track_descriptor_sptr,
    std::vector< vital::track_sptr > > desc_tuple_t;

  /// Set option to use object tracks for track descriptor history
  virtual void use_tracks_for_history( bool value ) = 0;

  /// Get a track descriptor by UID
  virtual bool get_track_descriptor(
    std::string const& uid,
    desc_tuple_t& result ) = 0;
};

/// Shared pointer for base queryable_track_set type
typedef std::shared_ptr< query_track_descriptor_set >
  query_track_descriptor_set_sptr;

} // namespace algo

} // namespace vital

}     // end namespace algo

#endif // VITAL_QUERY_TRACK_DESCRIPTOR_SET_H_
