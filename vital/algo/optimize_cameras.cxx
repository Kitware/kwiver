// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Instantiation of \link kwiver::vital::algo::algorithm_def
///        algorithm_def<T> \endlink for \link
///        kwiver::vital::algo::optimize_cameras optimize_cameras
///        \endlink

#include <vital/algo/algorithm.txx>
#include <vital/algo/optimize_cameras.h>

namespace kwiver {

namespace vital {

namespace algo {

optimize_cameras
::optimize_cameras()
{
  attach_logger( "algo.optimize_cameras" );
}

/// Optimize camera parameters given sets of landmarks and feature tracks
void
optimize_cameras
::optimize( camera_map_sptr& cameras,
            feature_track_set_sptr tracks,
            landmark_map_sptr landmarks,
            sfm_constraints_sptr constraints ) const
{
  if( !cameras || !tracks || !landmarks )
  {
    VITAL_THROW( invalid_value, "One or more input data pieces are Null!" );
  }
  typedef camera_map::map_camera_t map_camera_t;
  typedef landmark_map::map_landmark_t map_landmark_t;

  // extract data from containers
  map_camera_t cams = cameras->cameras();
  map_landmark_t lms = landmarks->landmarks();
  std::vector< track_sptr > trks = tracks->tracks();

  // Compose a map of frame IDs to a nested map of track ID to the state on
  // that frame number.
  typedef std::map< track_id_t, feature_sptr > inner_map_t;
  typedef std::map< frame_id_t, inner_map_t > states_map_t;

  states_map_t states_map;
  // O( len(trks) * avg_t_len )
  for( track_sptr const& t : trks )
  {
    // Only record a state if there is a corresponding landmark for the
    // track (constant-time check), the track state has a feature and thus a
    // location (constant-time check), and if we have a camera on the track
    // state's frame (constant-time check).
    if( lms.count( t->id() ) )
    {
      for( auto const& ts : *t )
      {
        auto fts = std::dynamic_pointer_cast< feature_track_state >( ts );
        if( fts && fts->feature && cams.count( ts->frame() ) )
        {
          states_map[ ts->frame() ][ t->id() ] = fts->feature;
        }
      }
    }
  }

  // For each camera in the input map, create corresponding point sets for 2D
  // and 3D coordinates of tracks and matching landmarks, respectively, for
  // that camera's frame.
  map_camera_t optimized_cameras;
  std::vector< feature_sptr > v_feat;
  std::vector< landmark_sptr > v_lms;

  for( map_camera_t::value_type const& p : cams )
  {
    v_feat.clear();
    v_lms.clear();

    // Construct 2d<->3d correspondences
    for( inner_map_t::value_type const& q : states_map[ p.first ] )
    {
      // Already guaranteed that feat and landmark exists above.
      v_feat.push_back( q.second );
      v_lms.push_back( lms[ q.first ] );
    }

    auto cam = std::dynamic_pointer_cast< camera_perspective >( p.second );
    this->optimize( cam, v_feat, v_lms, constraints );
    optimized_cameras[ p.first ] = cam;
  }

  cameras = camera_map_sptr( new simple_camera_map( optimized_cameras ) );
}

} // namespace algo

} // namespace vital

} // namespace kwiver

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::optimize_cameras );
/// \endcond
