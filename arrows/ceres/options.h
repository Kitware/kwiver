// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Internal header for helper classes containing Ceres options
 */

#ifndef KWIVER_ARROWS_CERES_CAMERA_OPTIONS_H_
#define KWIVER_ARROWS_CERES_CAMERA_OPTIONS_H_

#include <arrows/ceres/types.h>
#include <arrows/mvg/camera_options.h>
#include <vital/config/config_block.h>
#include <vital/types/sfm_constraints.h>
#include <vital/vital_config.h>

namespace kwiver {

namespace arrows {

namespace ceres {

/// Ceres solver options class

/**
 * The intended use of this class is for a PIMPL for an algorithm to
 * inherit from this class to share these options with that algorithm
 */
class solver_options
{
public:
  /// Constructor
  solver_options();

  /// Copy Constructor
  solver_options( const solver_options& other );

  /// populate the config block with options
  void get_configuration( vital::config_block_sptr config ) const;

  /// set the member variables from the config block
  void set_configuration( vital::config_block_sptr config );

  /// the Ceres solver options
  ::ceres::Solver::Options options;
};

/// Camera options class

/**
 * The intended use of this class is for a PIMPL for an algorithm to
 * inherit from this class to share these options with that algorithm
 */
struct camera_options : public mvg::camera_options
{
  camera_options(): mvg::camera_options() {}
  camera_options( const camera_options& other );
  virtual void get_configuration( config_block_sptr config ) const override;

  /// set the member variables from the config block
  virtual void set_configuration( config_block_sptr config ) override;

  /// Add the camera position priors costs to the Ceres problem.
  int
  add_position_prior_cost( ::ceres::Problem& problem,
                           cam_param_map_t& ext_params,
                           vital::sfm_constraints_sptr constraints );

  /// Add the camera intrinsic priors costs to the Ceres problem.
  void add_intrinsic_priors_cost(
    ::ceres::Problem& problem,
    std::vector< std::vector< double > >& int_params ) const;

  /// Add the camera path smoothness costs to the Ceres problem.
  void add_camera_path_smoothness_cost(
    ::ceres::Problem& problem,
    frame_params_t const& ordered_params ) const;

  /// Add the camera forward motion damping costs to the Ceres problem.
  void add_forward_motion_damping_cost(
    ::ceres::Problem& problem,
    frame_params_t const& ordered_params,
    cam_intrinsic_id_map_t const& frame_to_intr_map ) const;

  /**
   * Extract the extrinsic parameters from a camera into the parameter array.
   *
   *  \param [in]  camera The camera object to extract data from
   *  \param [out] params and array of 6 doubles to populate with parameters
   *
   *  This function is the inverse of update_camera_extrinsics.
   */
  void extract_camera_extrinsics( const camera_perspective_sptr camera,
                                  double* params ) const;
  /**
   * Extract the set of all unique intrinsic and extrinsic parameters from a
   * camera map.
   *
   *  \param [in]  cameras    The map of frame numbers to cameras to extract
   *parameters from
   *  \param [out] ext_params A map from frame number to vector of extrinsic
   *parameters
   *  \param [out] int_params A vector of unique camera intrinsic parameter
   *vectors
   *  \param [out] int_map    A map from frame number to index into \p
   *int_params.
   *                          The mapping may be many-to-one for shared
   *intrinsics.
   *
   *  This function is the inverse of update_camera_parameters.
   */
  void extract_camera_parameters( camera_map::map_camera_t const& cameras,
                                  cam_param_map_t& ext_params,
                                  std::vector< std::vector< double > >& int_params,
                                  cam_intrinsic_id_map_t& int_map ) const;

  /// the type of sharing of intrinsics between cameras to use
  CameraIntrinsicShareType camera_intrinsic_share_type = AUTO_SHARE_INTRINSICS;
};

} // namespace ceres

} // namespace arrows

} // namespace kwiver

#endif
