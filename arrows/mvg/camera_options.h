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

#include <vital/config/config_block.h>
#include <vital/types/camera_map.h>
#include <vital/types/camera_perspective.h>
#include <vital/vital_types.h>

#include <unordered_map>

namespace kwiver {

using namespace vital;

namespace arrows {

namespace mvg {

/// Default implementation of string options for cam enums
template < typename T >
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
  /// constructor
  camera_options();

  /// copy constructor
  camera_options( const camera_options& other );

  virtual ~camera_options(){};

  /// populate the config block with options
  virtual void get_configuration( config_block_sptr config ) const;

  /// set the member variables from the config block
  virtual void set_configuration( config_block_sptr config );

  /// Return true if any options to optimize intrinsic parameters are set.
  bool optimize_intrinsics() const;

  /// Update a camera object to use extrinsic parameters from an array

  /**
   *  \param [out] camera The simple_camera instance to update
   *  \param [in] params The array of 6 doubles to extract the data from
   *
   *  This function is the inverse of extract_camera_extrinsics
   */
  void update_camera_extrinsics(
    std::shared_ptr< simple_camera_perspective > camera,
    double const* params ) const;

  /// extract the parameters from camera intrinsics into the parameter array

  /**
   *  \param [in]  K The camera intrinsics object to extract data from
   *  \param [out] params and array of double to populate with parameters
   *
   *  \note the size of param is at least 5 but may be up to 12 depending
   *  on the number of distortion parameters used.
   *
   *  This function is the inverse of update_camera_intrinsics
   */
  void extract_camera_intrinsics( const camera_intrinsics_sptr K,
                                  double* params ) const;

  /// update the camera intrinsics from a parameter array

  /**
   *  \param [out] K The simple_camera_intrinsics instance to update
   *  \param [in] params The array of doubles to extract the data from
   *
   *  This function is the inverse of extract_camera_intrinsics
   */
  void update_camera_intrinsics( std::shared_ptr< simple_camera_intrinsics > K,
                                 const double* params ) const;

  /**
   * Enumerate the intrinsics held constant.
   *
   * Based on the settings of the boolean optimization switches
   * populate a vector of indices marking which intrinsic camera
   * parameters are held constant.  Indices are:
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
  std::vector< int > enumerate_constant_intrinsics() const;

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
  /// a soft lower bound on the horizontal field of view
  double minimum_hfov;
};

} // namespace mvg

} // namespace arrows

} // namespace kwiver

#define MVG_ENUM_HELPERS( NS, mvg_type ) \
  namespace kwiver { \
  namespace vital { \
\
  template <> \
  config_block_value_t \
  config_block_set_value_cast( NS::mvg_type const & value ); \
\
  template <> \
  NS::mvg_type \
  config_block_get_value_cast( config_block_value_t const & value ); \
\
  } \
\
  namespace arrows { \
  namespace mvg { \
\
  template <> \
  std::string \
  mvg_options< NS::mvg_type >(); \
\
  } \
  } \
  }

MVG_ENUM_HELPERS( kwiver::arrows::mvg, LensDistortionType )

#undef MVG_ENUM_HELPERS

#endif
