// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Internal header for helper classes containing Ceres options
 */

#ifndef KWIVER_ARROWS_CERES_CAMERA_OPTIONS_H_
#define KWIVER_ARROWS_CERES_CAMERA_OPTIONS_H_

#include <vital/vital_config.h>
#include <vital/config/config_block.h>
#include <vital/types/camera_perspective.h>
#include <vital/types/camera_map.h>
#include <vital/types/sfm_constraints.h>
#include <arrows/ceres/types.h>

#include <unordered_map>

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
  solver_options(const solver_options& other);

  /// populate the config block with options
  void get_configuration(vital::config_block_sptr config) const;

  /// set the member variables from the config block
  void set_configuration(vital::config_block_sptr config);

  /// the Ceres solver options
  ::ceres::Solver::Options options;
};

} // namespace ceres
} // namespace arrows
} // namespace kwiver

#endif
