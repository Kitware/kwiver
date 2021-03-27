// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief resection_camera instantiation
 */

#include <vital/algo/resection_camera.h>
#include <vital/algo/algorithm.txx>

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF(kwiver::vital::algo::resection_camera);
/// \endcond

using namespace std;

namespace kwiver {
namespace vital {
namespace algo {

resection_camera::resection_camera()
{
  attach_logger("algo.resection_camera");
}

camera_perspective_sptr
resection_camera::resection (frame_id_t const &frame,
			     landmark_map_sptr landmarks,
			     feature_track_set_sptr tracks,
			     camera_intrinsics_sptr cal) const
{
  std::vector<vector_2d> const pts2d;
  std::vector<vector_3d> const pts3d;
  // TODO: convert frame number, tracks, and landmarks into 2d and 3d points
  std::vector<bool> inliers;
  return resection(pts2d, pts3d, cal, inliers);
}

} } } // end namespace
