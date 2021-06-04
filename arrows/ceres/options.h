// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Internal header for helper classes containing Ceres options

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
///
/// PIMPL class should inherit from this class
/// to share these options with that algorithm.
class solver_options
{
public:
  solver_options() = default;
  solver_options( const solver_options& other ) = default;

  /// Populate the config block with options.
  void get_configuration( vital::config_block_sptr config ) const;

  /// Set the member variables from the config block.
  void set_configuration( vital::config_block_sptr config );

  /// Ceres solver options.
  ::ceres::Solver::Options options;
};

/// Camera options class
///
/// PIMPL class should inherit from this class
/// to share these options with that algorithm.
struct camera_options : public mvg::camera_options
{
  using cam_param_map_t =
    std::unordered_map< vital::frame_id_t, std::vector< double > >;
  using cam_intrinsic_id_map_t =
    std::unordered_map< vital::frame_id_t, unsigned int >;
  using frame_params_t =
    std::vector< std::pair< vital::frame_id_t, double* > >;

  camera_options() = default;
  camera_options( const camera_options& other );

  /// Populate the config block with options.
  virtual void get_configuration(
    vital::config_block_sptr config ) const override;

  /// Set the member variables from the config block.
  virtual void set_configuration( vital::config_block_sptr config ) override;

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

  /// Extract the extrinsic parameters from a camera into the parameter array.
  ///
  /// \param [in]  camera The camera object to extract data from
  /// \param [out] params and array of 6 doubles to populate with parameters
  ///
  /// This function is the inverse of update_camera_extrinsics.
  void extract_camera_extrinsics( const vital::camera_perspective_sptr camera,
                                  double* params ) const;
  /// Extract the set of all unique intrinsic and extrinsic parameters from a
  /// camera map.
  ///
  /// \param [in] cameras
  ///   The map of frame numbers to cameras to extract parameters from
  /// \param [out] ext_params
  ///   A map from frame number to vector of extrinsic parameters
  /// \param [out] int_params
  ///   A vector of unique camera intrinsic parameter vectors
  /// \param [out] int_map
  ///   A map from frame number to index into \p int_params.
  ///   The mapping may be many-to-one for shared intrinsics.
  ///
  ///  This function is the inverse of update_camera_parameters.
  void extract_camera_parameters(
    vital::camera_map::map_camera_t const& cameras,
    cam_param_map_t& ext_params,
    std::vector< std::vector< double > >& int_params,
    cam_intrinsic_id_map_t& int_map ) const;

  /// Update the camera objects using the extracted camera parameters.
  ///
  /// \param [out] cameras
  ///   The map of frame numbers to cameras to update
  /// \param [in] ext_params
  ///   A map from frame number to vector of extrinsic parameters
  /// \param [in] int_params
  ///   A vector of unique camera intrinsic parameter vectors
  /// \param [in] int_map
  ///   A map from frame number to index into \p int_params.
  ///   The mapping may be many-to-one for shared intrinsics.
  ///
  /// The original camera_intrinsic objects are reused if they were not
  /// optimized. Otherwise new camera_intrinsic instances are created.
  ///
  /// This function is the inverse of extract_camera_parameters.
  void
  update_camera_parameters( vital::camera_map::map_camera_t& cameras,
                            cam_param_map_t const& ext_params,
                            std::vector< std::vector< double > > const& int_params,
                            cam_intrinsic_id_map_t const& int_map ) const;

  /// Return true if any options to optimize intrinsic parameters are set.
  bool optimize_intrinsics() const;

  /// Update a camera object to use extrinsic parameters from an array.
  ///
  /// \param [out] camera The simple_camera instance to update
  /// \param [in] params The array of 6 doubles to extract the data from
  ///
  /// This function is the inverse of extract_camera_extrinsics.
  void update_camera_extrinsics(
    std::shared_ptr< vital::simple_camera_perspective > camera,
    double const* params ) const;

  /// Extract the parameters from camera intrinsics into the parameter array.
  ///
  /// \param [in]  K The camera intrinsics object to extract data from
  /// \param [out] params and array of double to populate with parameters
  ///
  /// \note the size of param is at least 5 but may be up to 12 depending
  ///  on the number of distortion parameters used.
  ///
  /// This function is the inverse of update_camera_intrinsics.
  void extract_camera_intrinsics( const vital::camera_intrinsics_sptr K,
                                  double* params ) const;

  /// Update the camera intrinsics from a parameter array.
  ///
  /// \param [out] K The simple_camera_intrinsics instance to update
  /// \param [in] params The array of doubles to extract the data from
  ///
  /// This function is the inverse of extract_camera_intrinsics.
  void update_camera_intrinsics(
    std::shared_ptr< vital::simple_camera_intrinsics > K,
    const double* params ) const;

  /// Enumerate the intrinsics held constant.
  ///
  /// Based on the settings of the boolean optimization switches
  /// populate a vector of indices marking which intrinsic camera
  /// parameters are held constant.  Indices are:
  ///   - \b 0 : focal length
  ///   - \b 1 : principal point X
  ///   - \b 2 : principal point Y
  ///   - \b 3 : aspect ratio
  ///   - \b 4 : skew
  ///   - \b 5 : radial distortion (k1)
  ///   - \b 6 : radial distortion (k2)
  ///   - \b 7 : tangential distortion (p1)
  ///   - \b 8 : tangential distortion (p2)
  ///   - \b 9 : radial distortion (k3)
  ///   - \b 10 : radial distortion (k4)
  ///   - \b 11 : radial distortion (k5)
  ///   - \b 12 : radial distortion (k6)
  std::vector< int > enumerate_constant_intrinsics() const;

  /// type of sharing of intrinsics between cameras to use
  CameraIntrinsicShareType camera_intrinsic_share_type = AUTO_SHARE_INTRINSICS;
  /// amount of the camera path smoothness regularization
  double camera_path_smoothness = 0.0;
  /// scale of camera forward motion damping regularization
  double camera_forward_motion_damping = 0.0;
};

/// Return the number of distortion parameters required for each type.
KWIVER_ALGO_CERES_EXPORT
unsigned int
num_distortion_params( mvg::LensDistortionType type );

} // namespace ceres

} // namespace arrows

} // namespace kwiver

#endif
