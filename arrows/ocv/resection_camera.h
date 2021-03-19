// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief OCV resection_camera algorithm impl interface
 */

#ifndef KWIVER_ARROWS_OCV_RESECTION_CAMERA_H_
#define KWIVER_ARROWS_OCV_RESECTION_CAMERA_H_

#include <arrows/ocv/kwiver_algo_ocv_export.h>
#include <vital/algo/resection_camera.h>
#include <vital/vital_config.h>

namespace kwiver {

namespace arrows {

namespace ocv {

/// Use OpenCV to estimate a camera's pose and projection matrix from 3D
/// feature and point projection pairs.
class KWIVER_ALGO_OCV_EXPORT resection_camera
  : public vital::algo::resection_camera
{
public:
  PLUGIN_INFO( "ocv",
               "resection camera using OpenCV calibrate camera method" )
  /// Instantiate.
  resection_camera();
  /// Destroy.
  virtual ~resection_camera();

  /// Get this algorithm's \link vital::config_block configuration block
  /// \endlink.
  virtual vital::config_block_sptr get_configuration() const;

  /// Set this algorithm's properties via a config block.
  virtual void set_configuration( vital::config_block_sptr config );

  /// Check that the algorithm's configuration config_block is valid.
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  virtual kwiver::vital::camera_perspective_sptr
  resection(
    std::vector< kwiver::vital::vector_2d > const& pts2d,
    std::vector< kwiver::vital::vector_3d > const& pts3d,
    std::vector< bool >& inliers,
    kwiver::vital::camera_intrinsics_sptr cal ) const override;

  /// Estimate camera parameters for a frame from landmarks and tracks.
  /// This is a convenience function, constructing the pts2d and pts3d from a frame, landmarks, and tracks, and then call resection() with the point correspondences.
  /// \return estimated camera parameters
  virtual kwiver::vital::camera_perspective_sptr
  resection (kwiver::vital::frame_id_t const &frame, ///< [in]  frame number for which to estimate a camera
      kwiver::vital::landmark_map_sptr landmarks, ///< [in]  3D landmarks locations to constrain camera
      kwiver::vital::feature_track_set_sptr tracks, ///< [in]  2D feature tracks in image coordinates
      kwiver::vital::camera_intrinsics_sptr cal = nullptr ///< [in] initial guess intrinsic parameters of the camera
  ) const override;

private:
  /// private implementation
  class priv;
  const std::unique_ptr< priv > d_;
};

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif
