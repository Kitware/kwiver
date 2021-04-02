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

resection_camera
::resection_camera()
{
  attach_logger( "algo.resection_camera" );
}

camera_perspective_sptr
resection_camera
::resection( frame_id_t frmID,
             landmark_map_sptr landmarks,
             feature_track_set_sptr tracks,
             unsigned width, unsigned height ) const
{
  vector< vector_3d > pts_3d; // world points
  vector< vector_2d > pts_projs; // corresponding image points
  for( auto const& track : tracks->tracks() )
  {
    auto lm_id = track->id();
    auto lm_it = landmarks->landmarks().find( lm_id );
    pts_3d.push_back( lm_it->second->loc() );
    auto const fts =
      dynamic_pointer_cast< feature_track_state >( *track->find( frmID ) );
    pts_projs.push_back( fts->feature->loc() );
  }
  // resection camera using point correspondences and image dimensions
  const vector_2d principal_point( width / 2., height / 2. );
  camera_intrinsics_sptr cal(
    new simple_camera_intrinsics( ( width + height ) / 2., principal_point,
                                  1.0, 0.0,
                                  Eigen::VectorXd(), width, height ) );
  vector< bool > inliers;
  return resection( pts_projs, pts_3d, inliers, cal );
}

camera_perspective_sptr
resection_camera
::resection( frame_id_t frmID,
             landmark_map_sptr landmarks,
             feature_track_set_sptr tracks,
	       kwiver::vital::camera_intrinsics_sptr cal ) const
{
  vector< vector_3d > pts_3d; // world points
  vector< vector_2d > pts_projs; // corresponding image points
  for( auto const& track : tracks->tracks() )
  {
    auto lm_id = track->id();
    auto lm_it = landmarks->landmarks().find( lm_id );
    pts_3d.push_back( lm_it->second->loc() );
    auto const fts =
      dynamic_pointer_cast< feature_track_state >( *track->find( frmID ) );
    pts_projs.push_back( fts->feature->loc() );
  }
  // resection camera using point correspondences and initial calibration guess
  vector< bool > inliers;
  return resection( pts_projs, pts_3d, inliers, cal );
}

} // end algo

} // end vital

} // end kwiver
