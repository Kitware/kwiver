// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Header for MVG camera options.
 */

#ifndef KWIVER_ARROWS_MVG_CAMERA_OPTIONS_H_
#define KWIVER_ARROWS_MVG_CAMERA_OPTIONS_H_

#include <arrows/ceres/lens_distortion.h>
#include <arrows/mvg/kwiver_algo_mvg_export.h>

#include <vital/config/config_block.h>
#include <vital/types/camera_map.h>
#include <vital/types/camera_perspective.h>
#include <vital/vital_types.h>

#include <unordered_map>

namespace kwiver {

namespace arrows {

namespace mvg {

/// various models for lens distortion supported in the config
enum LensDistortionType
{
  NO_DISTORTION,
  POLYNOMIAL_RADIAL_DISTORTION,
  POLYNOMIAL_RADIAL_TANGENTIAL_DISTORTION,
  RATIONAL_RADIAL_TANGENTIAL_DISTORTION
};

/// Provide a string representation for a LensDisortionType value.
KWIVER_ALGO_MVG_EXPORT
const char*
LensDistortionTypeToString(LensDistortionType type);

/// Parse a LensDistortionType value from a string or return false.
KWIVER_ALGO_MVG_EXPORT
bool
StringToLensDistortionType(std::string value, LensDistortionType* type);

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
  virtual void get_configuration( vital::config_block_sptr config ) const;

  /// set the member variables from the config block
  virtual void set_configuration( vital::config_block_sptr config );

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
