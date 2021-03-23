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

namespace kwiver {
namespace vital {
namespace algo {

resection_camera::resection_camera()
{
  attach_logger( "algo.resection_camera" );
}

//kwiver::vital::camera_perspective_sptr
//resection_camera::resection(
//	kwiver::vital::frame_id_t const& frame, ///< [in]  frame number for which to estimate a camera
//    kwiver::vital::landmark_map_sptr landmarks, ///< [in]  3D landmarks locations to constrain camera
//    kwiver::vital::feature_track_set_sptr tracks, ///< [in]  2D feature tracks in image coordinates
//    kwiver::vital::camera_intrinsics_sptr init_cal ///< [in]  initial guess intrinsic parameters of the camera
//) const
//{
//	kwiver::vital::camera_perspective_sptr res;
//	// TODO: implement
//	return res;
//}


} } } // end namespace
