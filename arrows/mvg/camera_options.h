// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Header for MVG camera options.
 */

#ifndef KWIVER_ARROWS_MVG_CAMERA_OPTIONS_H_
#define KWIVER_ARROWS_MVG_CAMERA_OPTIONS_H_

#include "lens_distortion.h"

#include <vital/vital_types.h>
#include <vital/config/config_block.h>
#include <vital/types/camera_map.h>
#include <vital/types/camera_perspective.h>

#include <unordered_map>

namespace kwiver {

using namespace vital;

namespace arrows {
namespace mvg {

/// The options for camera intrinsic sharing supported in the config
enum CameraIntrinsicShareType
{
  AUTO_SHARE_INTRINSICS,
  FORCE_COMMON_INTRINSICS,
  FORCE_UNIQUE_INTRINSICS
};

/// Provide a string representation for a CameraIntrinsicShareType value
//KWIVER_ALGO_MVG_EXPORT
const char*
CameraIntrinsicShareTypeToString(CameraIntrinsicShareType type);

/// Parse a CameraIntrinsicShareType value from a string or return false
//KWIVER_ALGO_MVG_EXPORT
bool
StringToCameraIntrinsicShareType(std::string value, CameraIntrinsicShareType* type);

/// Default implementation of string options for cam enums
template <typename T>
std::string
mvg_options()
{
  return std::string();
}

/// Camera options class
/**
 * The intended use of this class is for a PIMPL for an algorithm to
 * inherit from this class to share these options with that algorithm
 */
struct KWIVER_ALGO_MVG_EXPORT camera_options
{
  /// typedef for camera parameter map
  typedef std::unordered_map<frame_id_t, std::vector<double> > cam_param_map_t;
  typedef std::unordered_map<frame_id_t, unsigned int> cam_intrinsic_id_map_t;
  typedef std::vector<std::pair<frame_id_t, double *> > frame_params_t;

  /// Constructor
  camera_options();

  /// Copy Constructor
  camera_options(const camera_options& other);

  /// populate the config block with options
  void get_configuration(config_block_sptr config) const;

  /// set the member variables from the config block
  void set_configuration(config_block_sptr config);

  /// Return true if any options to optimize intrinsic parameters are set
  bool optimize_intrinsics() const;

  /// extract the extrinsic parameters from a camera into the parameter array
  /**
   *  \param [in]  camera The camera object to extract data from
   *  \param [out] params and array of 6 doubles to populate with parameters
   *
   *  This function is the inverse of update_camera_extrinsics
   */
  void extract_camera_extrinsics(const camera_perspective_sptr camera,
                                 double* params) const;

  /// Update a camera object to use extrinsic parameters from an array
  /**
   *  \param [out] camera The simple_camera instance to update
   *  \param [in] params The array of 6 doubles to extract the data from
   *
   *  This function is the inverse of extract_camera_extrinsics
   */
  void update_camera_extrinsics(
    std::shared_ptr<simple_camera_perspective> camera,
    double const* params) const;

  /// extract the paramters from camera intrinsics into the parameter array
  /**
   *  \param [in]  K The camera intrinsics object to extract data from
   *  \param [out] params and array of double to populate with parameters
   *
   *  \note the size of param is at least 5 but may be up to 12 depending
   *  on the number of distortion parameters used.
   *
   *  This function is the inverse of update_camera_intrinsics
   */
  void extract_camera_intrinsics(const camera_intrinsics_sptr K,
                                 double* params) const;

  /// update the camera intrinsics from a parameter array
  /**
   *  \param [out] K The simple_camera_intrinsics instance to update
   *  \param [in] params The array of doubles to extract the data from
   *
   *  This function is the inverse of extract_camera_intrinsics
   */
  void update_camera_intrinsics(std::shared_ptr<simple_camera_intrinsics> K,
                                const double* params) const;

  /// extract the set of all unique intrinsic and extrinsic parameters from a camera map
  /**
   *  \param [in]  cameras    The map of frame numbers to cameras to extract parameters from
   *  \param [out] ext_params A map from frame number to vector of extrinsic parameters
   *  \param [out] int_params A vector of unique camera intrinsic parameter vectors
   *  \param [out] int_map    A map from frame number to index into \p int_params.
   *                          The mapping may be many-to-one for shared intrinsics.
   *
   *  This function is the inverse of update_camera_parameters
   */
  void extract_camera_parameters(camera_map::map_camera_t const& cameras,
                                 cam_param_map_t& ext_params,
                                 std::vector<std::vector<double> >& int_params,
                                 cam_intrinsic_id_map_t& int_map) const;

  /// update the camera objects using the extracted camera parameters
  /**
   *  \param [out] cameras    The map of frame numbers to cameras to update
   *  \param [in]  ext_params A map from frame number to vector of extrinsic parameters
   *  \param [in]  int_params A vector of unique camera intrinsic parameter vectors
   *  \param [in]  int_map    A map from frame number to index into \p int_params.
   *                          The mapping may be many-to-one for shared intrinsics.
   *
   *  The original camera_intrinsic objects are reused if they were not optimized.
   *  Otherwise new camera_intrinsic instances are created.
   *
   *  This function is the inverse of extract_camera_parameters
   */
  void
  update_camera_parameters(camera_map::map_camera_t& cameras,
                           cam_param_map_t const& ext_params,
                           std::vector<std::vector<double> > const& int_params,
                           cam_intrinsic_id_map_t const& int_map) const;

  /// enumerate the intrinsics held constant
  /**
   * Based on the settings of the boolean optimization switches
   * poplulate a vector of indices marking which intrinsic camera
   * paramaters are held constant.  Indices are:
   *   - \b 0 : focal length
   *   - \b 1 : principal point X
   *   - \b 2 : principal point Y
   *   - \b 3 : aspect ratio
   *   - \b 4 : skew
   *   - \b 5 : radial distortion (k1)
   *   - \b 6 : radial distortion (k2)
   *   - \b 7 : tangential distortion (p1)
   *   - \b 8 : tangential distortion (p2)
   *   - \b 9 : radial distortion (k3)
   *   - \b 10 : radial distortion (k4)
   *   - \b 11 : radial distortion (k5)
   *   - \b 12 : radial distortion (k6)
   */
  std::vector<int> enumerate_constant_intrinsics() const;

  /// option to optimize the focal length
  bool optimize_focal_length;
  /// option to optimize aspect ratio
  bool optimize_aspect_ratio;
  /// option to optimize principal point
  bool optimize_principal_point;
  /// option to optimize skew
  bool optimize_skew;
  /// the lens distortion model to use
  LensDistortionType lens_distortion_type;
  /// option to optimize radial distortion parameter k1
  bool optimize_dist_k1;
  /// option to optimize radial distortion parameter k2
  bool optimize_dist_k2;
  /// option to optimize radial distortion parameter k3
  bool optimize_dist_k3;
  /// option to optimize tangential distortions parameters p1, p2
  bool optimize_dist_p1_p2;
  /// option to optimize radial distortion parameters k4, k5, k6
  bool optimize_dist_k4_k5_k6;
  /// the type of sharing of intrinsics between cameras to use
  CameraIntrinsicShareType camera_intrinsic_share_type;
  /// the amount of the camera path smoothness regularization
  double camera_path_smoothness;
  /// the scale of camera forward motion damping regularization
  double camera_forward_motion_damping;
  /// a soft lower bound on the horizontal field of view
  double minimum_hfov;
};

} // namespace mvg
} // namespace arrows
} // namespace kwiver

#define MVG_ENUM_HELPERS(NS, mvg_type) \
namespace kwiver { \
namespace vital { \
\
template<> \
config_block_value_t \
config_block_set_value_cast(NS::mvg_type const& value); \
\
template<> \
NS::mvg_type \
config_block_get_value_cast(config_block_value_t const& value); \
\
}\
\
namespace arrows { \
namespace mvg { \
\
template<> \
std::string \
mvg_options< NS::mvg_type >(); \
\
}\
}\
}

MVG_ENUM_HELPERS(kwiver::arrows::mvg, LensDistortionType)
MVG_ENUM_HELPERS(kwiver::arrows::mvg, CameraIntrinsicShareType)

#undef MVG_ENUM_HELPERS

#endif
