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

kwiver::vital::camera_perspective_sptr
resection_camera::resect(kwiver::vital::frame_id_t const & frame,
          kwiver::vital::landmark_map_sptr landmarks,
          kwiver::vital::feature_track_set_sptr tracks,
          kwiver::vital::camera_intrinsics_sptr cal
) const
{
	kwiver::vital::camera_perspective_sptr res;
	// TODO: implement
	return res;
}


} } } // end namespace
