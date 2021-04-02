// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief resection_camera instantiation

#include <vital/algo/algorithm.txx>
#include <vital/algo/resection_camera.h>

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::resection_camera );
/// \endcond

using namespace std;

namespace kwiver {

namespace vital {

namespace algo {

// ----------------------------------------------------------------------------
resection_camera
::resection_camera()
{
  attach_logger( "algo.resection_camera" );
}

void get_points(const frame_id_t frmID,
  const landmark_map_sptr landmarks, const feature_track_set_sptr tracks,
  vector< vector_3d > & pts_3d, vector< vector_2d > & pts_projs)
{
  auto lmx = landmarks->landmarks();
  for( auto const& track : tracks->tracks() )
  {
    auto lm_id = track->id();
    auto lm_it = lmx.find( lm_id );
    if (lm_it == lmx.end()) continue;
    auto tr_id = track->find( frmID );
    if (tr_id == track->end()) continue;
    auto const fts =
      dynamic_pointer_cast< feature_track_state >( *track->find( frmID ) );
    pts_3d.emplace_back( lm_it->second->loc() );
    pts_projs.emplace_back( fts->feature->loc() );
  }
}

camera_perspective_sptr
resection_camera
::resection( frame_id_t frame,
             landmark_map_sptr landmarks,
             feature_track_set_sptr tracks,
             unsigned width, unsigned height ) const
{
  // Generate calibration guess from image dimensions.
  auto const principal_point = vector_2d{ width * 0.5, height * 0.5 };
  auto cal = std::make_shared< simple_camera_intrinsics >(
    ( width + height ) * 0.5, principal_point, 1.0, 0.0,
    Eigen::VectorXd(), width, height );

  // Resection using guessed calibration.
  return resection( frame_id, landmarks, tracks, cal );
}

camera_perspective_sptr
resection_camera
::resection( frame_id_t frmID,
             landmark_map_sptr landmarks,
             feature_track_set_sptr tracks,
             kwiver::vital::camera_intrinsics_sptr cal ) const
{
  auto world_points = vector< vector_3d >{};
  auto camera_points = vector< vector_2d >{};

  auto const& real_landmarks = landmarks->landmarks();
  for( auto const& fts : tracks->frame_feature_track_states(frame_id) )
  {
    auto lmi = real_landmarks.find( fts->track()->id() );
    if( lmi != real_landmarks.end() )
    {
      world_points.emplace_back( lmi->second->loc() );
      camera_points.emplace_back( fts->feature->loc() );
    }
  }

  // Resection camera using point correspondences and initial calibration guess.
  vector< bool > inliers;
  return resection( pts_projs, pts_3d, inliers, cal );
}

} // namespace algo

} // namespace vital

} // namespace kwiver
