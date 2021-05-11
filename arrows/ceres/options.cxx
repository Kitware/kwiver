// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation of Ceres options helper classes
 */

#include "options.h"

#include <arrows/ceres/camera_smoothness.h>
#include <arrows/ceres/camera_position.h>
#include <arrows/ceres/camera_intrinsic_prior.h>

#include <vital/math_constants.h>

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace ceres {

/// Constructor
solver_options
::solver_options()
  : options()
{
}

/// Copy Constructor
solver_options
::solver_options(const solver_options& other)
  : options( other.options )
{
}

/// populate the config block with options
void
solver_options
::get_configuration(config_block_sptr config) const
{
  ::ceres::Solver::Options const& o = this->options;
  config->set_value("num_threads", o.num_threads,
                    "Number of threads to use");
  config->set_value("num_linear_solver_threads", o.num_linear_solver_threads,
                    "Number of threads to use in the linear solver");
  config->set_value("max_num_iterations", o.max_num_iterations,
                    "Maximum number of iteration of allow");
  config->set_value("function_tolerance", o.function_tolerance,
                    "Solver terminates if relative cost change is below this "
                    "tolerance");
  config->set_value("gradient_tolerance", o.gradient_tolerance,
                    "Solver terminates if the maximum gradient is below this "
                    "tolerance");
  config->set_value("parameter_tolerance", o.parameter_tolerance,
                    "Solver terminates if the relative change in parameters "
                    "is below this tolerance");
  config->set_value("linear_solver_type", o.linear_solver_type,
                    "Linear solver to use."
                    + ceres_options< ::ceres::LinearSolverType >());
  config->set_value("preconditioner_type", o.preconditioner_type,
                    "Preconditioner to use."
                    + ceres_options< ::ceres::PreconditionerType >());
  config->set_value("trust_region_strategy_type", o.trust_region_strategy_type,
                    "Trust region step compution algorithm used by Ceres."
                    + ceres_options< ::ceres::TrustRegionStrategyType >());
  config->set_value("dogleg_type", o.dogleg_type,
                    "Dogleg strategy to use."
                    + ceres_options< ::ceres::DoglegType >());
  config->set_value("update_state_every_iteration", o.update_state_every_iteration,
                    "If true, the updated state is computed and provided in "
                    "the callback on every iteration.  This slows down "
                    "optimization but can be useful for debugging.");
}

/// set the member variables from the config block
void
solver_options
::set_configuration(config_block_sptr config)
{
#define GET_VALUE(vtype, vname) \
  options.vname = config->get_value< vtype >(#vname, options.vname);

  GET_VALUE(int,    num_threads);
  GET_VALUE(int,    num_linear_solver_threads);
  GET_VALUE(int,    max_num_iterations);
  GET_VALUE(double, function_tolerance);
  GET_VALUE(double, gradient_tolerance);
  GET_VALUE(double, parameter_tolerance);
  GET_VALUE(::ceres::LinearSolverType, linear_solver_type);
  GET_VALUE(::ceres::PreconditionerType, preconditioner_type);
  GET_VALUE(::ceres::TrustRegionStrategyType, trust_region_strategy_type);
  GET_VALUE(::ceres::DoglegType, dogleg_type);
  GET_VALUE(bool, update_state_every_iteration);

#undef GET_VALUE
}

} // end namespace ceres
} // end namespace arrows
} // end namespace kwiver
