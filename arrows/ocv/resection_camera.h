// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief OCV resection_camera algorithm impl interface
 */

#ifndef KWIVER_ARROWS_OCV_RESECTION_CAMERA_H_
#define KWIVER_ARROWS_OCV_RESECTION_CAMERA_H_

#include <vital/vital_config.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <vital/algo/resection_camera.h>

namespace kwiver {
namespace arrows {
namespace ocv {

/// Use OpenCV to estimate a camera's pose and projection matrix from 3D feature
/// and point projection pairs.
class KWIVER_ALGO_OCV_EXPORT resection_camera
  : public vital::algo::resection_camera
{
public:
  PLUGIN_INFO( "ocv",
               "Estimate camera pose and projection matrix with perspective N point method")

  /// Instantiate.
  resection_camera();

  /// Destroy.
  virtual ~resection_camera();

  /// Get this algorithm's \link vital::config_block configuration block \endlink.
  virtual vital::config_block_sptr get_configuration() const;

  /// Set this algorithm's properties via a config block.
  virtual void set_configuration(vital::config_block_sptr config);

  /// Check that the algorithm's configuration config_block is valid.
  virtual bool check_configuration(vital::config_block_sptr config) const;

  /// Estimate the camera's pose and projection matrix from the 3D points and their corresponding projections.
  /// \return estimated camera parameters
  virtual
  kwiver::vital::camera_perspective_sptr
  resection(const std::vector<vital::vector_2d>& pts2d, ///< [in]  2d projections of pts3d in the same order as pts3d
           const std::vector<vital::vector_3d>& pts3d, ///< [in]  3d landmarks in the same order as pts2d, assuming a 1-1 correspondence
           std::vector<bool>& inliers, ///< [out] inlier flags for each point, the value is true if this pair is an inlier to the estimate
		   const kwiver::vital::camera_intrinsics_sptr cal = nullptr ///< [in]  optional initial intrinsic parameters for the camera
  ) const override;

private:
  /// private implementation class
  class priv;
  const std::unique_ptr<priv> d_;
};

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver

#endif
