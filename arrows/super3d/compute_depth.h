// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header file for compute depth

#ifndef KWIVER_ARROWS_SUPER3D_COMPUTE_DEPTH_H_
#define KWIVER_ARROWS_SUPER3D_COMPUTE_DEPTH_H_

#include <arrows/super3d/kwiver_algo_super3d_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/compute_depth.h>
#include <vital/types/vector.h>
#include <vital/vital_config.h>
#include <vnl/vnl_double_3.h>

namespace kwiver {

namespace arrows {

namespace super3d {

/// A class for depth map estimation
class KWIVER_ALGO_SUPER3D_EXPORT compute_depth
  : public vital::algo::compute_depth
{
public:
  PLUGGABLE_IMPL(
    compute_depth,
    "Compute depth maps from image sequences, using vxl",
    PARAM_DEFAULT(
      iterations, int,
      "Number of iterations to run optimizer", 2000 ),
    PARAM_DEFAULT(
      theta0, double,
      "Begin value of quadratic relaxation term", 1.0 ),
    PARAM_DEFAULT(
      theta_end, double,
      "End value of quadratic relaxation term", 0.001 ),
    PARAM_DEFAULT(
      lambda, double,
      "Weight of the data term", 0.65 ),
    PARAM_DEFAULT(
      gw_alpha, double,
      "gradient weighting term", 20 ),
    PARAM_DEFAULT(
      epsilon, double,
      "Huber norm term, trade off between L1 and L2 norms", 0.01 ),
    PARAM_DEFAULT(
      world_plane_normal, vnl_double_3,
      "up direction in world space", vnl_double_3( 0, 0, 1 ) ),
    PARAM_DEFAULT(
      callback_interval, int,
      "number of iterations between updates (-1 turns off updates)", -1 ),
    PARAM_DEFAULT(
      uncertainty_in_callback, bool,
      "If true, compute the uncertainty in each callback for a "
      "live preview at additional computational cost. "
      "Otherwise, uncertainty is only computed at the end.", false ),
    PARAM_DEFAULT(
      depth_sample_rate, double,
      "Specifies the maximum sampling rate, in pixels, of the "
      "depth steps projected into support views.  This rate "
      "determines the number of depth slices in the cost "
      "volume.  Smaller values create more depth slices.", 0.5 )
  )
  /// Destructor
  virtual ~compute_depth() = default;

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Compute a depth map from an image sequence and return uncertainty by ref
  ///
  /// Implementations of this function should not modify the underlying objects
  /// contained in the input structures. Output references should either be new
  /// instances or the same as input.
  ///
  /// \param [in] frames image sequence to compute depth with
  /// \param [in] cameras corresponding to the image sequence
  /// \param [in] depth_min minimum depth expected
  /// \param [in] depth_max maximum depth expected
  /// \param [in] reference_frame index into image sequence denoting the frame
  /// that depth is computed on
  /// \param [in] roi region of interest within reference image (can be entire
  /// image)
  /// \param [out] depth_uncertainty reference which will contain depth
  /// uncertainty
  /// \param [in] masks optional masks corresponding to the image sequence
  virtual kwiver::vital::image_container_sptr
  compute(
    std::vector< kwiver::vital::image_container_sptr > const& frames,
    std::vector< kwiver::vital::camera_perspective_sptr > const& cameras,
    double depth_min, double depth_max,
    size_t reference_frame,
    vital::bounding_box< int > const& roi,
    kwiver::vital::image_container_sptr& depth_uncertainty,
    std::vector< kwiver::vital::image_container_sptr > const& masks =
    std::vector< kwiver::vital::image_container_sptr >( ) ) const;

  /// Set callback for receiving incremental updates
  virtual void set_callback( callback_t cb );

protected:
  void initialize() override;

private:
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

}  // end namespace super3d

}  // end namespace arrows

}  // end namespace kwiver

#endif
