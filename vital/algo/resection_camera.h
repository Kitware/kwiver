// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief resection_camera algorithm definition

#ifndef VITAL_ALGO_RESECTION_CAMERA_H_
#define VITAL_ALGO_RESECTION_CAMERA_H_

#include <vital/algo/algorithm.h>
#include <vital/types/camera_perspective.h>
#include <vital/types/feature_track_set.h>
#include <vital/types/landmark_map.h>

#include <vector>

namespace kwiver {

namespace vital {

/// An abstract base class to resection a camera using 3D feature and point projection pairs.
class VITAL_ALGO_EXPORT resection_camera
  : public kwiver::vital::algorithm_def< resection_camera >
{
public:
  /// \return name of this algorithm
  static std::string
  static_type_name() { return "resection_camera"; }

  /// Estimate camera parameters from 3D points and their corresponding
  /// projections.
  ///
  /// \param [in] pts2d 2d projections of pts3d points
  /// \param [in] pts3d 3d points in a 1-1 correspondence with pts2d
  /// \param [out] inliers inlier flags for the point pairs
  /// \param [in] cal initial guess on intrinsic parameters of the camera
  /// \return estimated camera parameters
  virtual
  kwiver::vital::camera_perspective_sptr
  resection(
	  std::vector<kwiver::vital::vector_2d> const & pts2d, ///< [in]  2d projections of pts3d in the same order as pts3d
	  std::vector<kwiver::vital::vector_3d> const & pts3d, ///< [in]  3d points in the same order as pts2d, assuming a 1-1 correspondence
      std::vector<bool>& inliers, ///< [out] inlier flags for each point, the value is true if this pair is an inlier to the estimate
      kwiver::vital::camera_intrinsics_sptr cal = nullptr ///< [in] optional guess on intrinsic parameters of the camera, [out] refined intrinsic parameters of the camera
  ) const = 0;

  /// Estimate camera parameters for a frame from landmarks and tracks.
  ///
  /// This is a convenience function, callin internally
  /// resection(pts2d, pts3d, ...) with the recoverd point correspondences.
  ///
  /// \param [in] frmID frame number for which to estimate a camera
  /// \param [in] landmarks 3D landmark locations to constrain camera
  /// \param [in] tracks 2D feature tracks in image coordinates
  /// \param [in] width image size in the x dimension in pixels
  /// \param [in] height image size in the y dimension in pixels
  /// \return estimated camera parameters
  virtual
  kwiver::vital::camera_perspective_sptr
  resection(kwiver::vital::frame_id_t const & frame, ///< [in]  frame number for which to estimate a camera
            kwiver::vital::landmark_map_sptr landmarks, ///< [in]  3D landmarks locations to constrain camera
            kwiver::vital::feature_track_set_sptr tracks, ///< [in]  2D feature tracks in image coordinates
            kwiver::vital::camera_intrinsics_sptr cal = nullptr ///< [in] initial guess intrinsic parameters of the camera, [out] refined intrinsic parameters of the camera
  ) const;

protected:
  resection_camera();
};

/// Shared pointer type of base resection_camera algorithm definition class.
using resection_camera_sptr = std::shared_ptr< resection_camera >;

} // namespace algo

} // namespace vital

} // namespace kwiver

#endif
