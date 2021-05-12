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

int
camera_options
::add_position_prior_cost(
  ::ceres::Problem& problem,
  cam_param_map_t& ext_params,
  sfm_constraints_sptr constraints)
{
  int num_priors_applied = 0;
  if (!constraints)
  {
    return num_priors_applied;
  }

  ::ceres::LossFunction *loss = LossFunctionFactory(SOFT_L_ONE_LOSS, 100.0);

  for (auto& ext_par : ext_params)
  {
    auto fid = ext_par.first;
    vector_3d position_prior_local;
    if(!constraints->get_camera_position_prior_local(fid,position_prior_local))
    {
      continue;
    }

    double *cam_state = &ext_par.second[0];
    if (!problem.HasParameterBlock(cam_state))
    {
      continue;
    }
    if (problem.IsParameterBlockConstant(cam_state))
    {
	continue;
    }

    auto position_prior_cost =
      camera_position::create(position_prior_local);

    problem.AddResidualBlock(position_prior_cost,
      loss,
      cam_state);
    ++num_priors_applied;
  }
  return num_priors_applied;
}

/// Add the camera intrinsic priors costs to the Ceres problem
void
camera_options
::add_intrinsic_priors_cost(
  ::ceres::Problem& problem,
  std::vector<std::vector<double> >& int_params) const
{
  if (this->minimum_hfov <= 0.0)
  {
    return;
  }
  // scaling balances the response based on the number of other residuals.
  double scale = static_cast<double>(std::max(1, problem.NumResiduals()));
  auto scaled_loss = new ::ceres::ScaledLoss(NULL, scale,
			   ::ceres::Ownership::TAKE_OWNERSHIP);
  for (auto& int_par : int_params)
  {
    // assume image width is twice the principal point X coordinate
    double width = 2.0 * int_par[1];
    if (width <= 0.0)
    {
      width = 1280.0;
    }
    double max_focal_len = width / (2.0 * std::tan(this->minimum_hfov * deg_to_rad / 2.0));
    auto cam_intrin_prior_cost =
      camera_intrinsic_prior::create(max_focal_len, int_par.size());

    double* foc_len = &int_par[0];
    // add the loss with squared error
    problem.AddResidualBlock(cam_intrin_prior_cost,
			     scaled_loss,
			     foc_len);
  }
}

/// Add the camera path smoothness costs to the Ceres problem
void
camera_options
::add_camera_path_smoothness_cost(
    ::ceres::Problem& problem,
    frame_params_t const& ordered_params) const
{
  // Add camera path regularization residuals
  if( this->camera_path_smoothness > 0.0 &&
      ordered_params.size() >= 3 )
  {
    double sum_dist = 0.0;
    auto prev_cam = ordered_params.begin();
    auto curr_cam = prev_cam;
    curr_cam++;
    auto next_cam = curr_cam;
    next_cam++;
    std::vector<std::tuple<decltype(curr_cam), double, double> > constraints;
    for(; next_cam != ordered_params.end();
	prev_cam = curr_cam, curr_cam = next_cam, next_cam++)
    {
      double inv_dist = 1.0 / static_cast<double>(next_cam->first -
						  prev_cam->first);
      double frac = (curr_cam->first - prev_cam->first) * inv_dist;
      if (inv_dist > 0.0 && frac > 0.0 && frac < 1.0 )
      {
	sum_dist += (Eigen::Map<vector_3d>(prev_cam->second+3) -
		     Eigen::Map<vector_3d>(next_cam->second+3)).norm();
	constraints.push_back(std::make_tuple(curr_cam, inv_dist, frac));
      }
    }
    // Normalize the weight
    const double weight = this->camera_path_smoothness
			* problem.NumResiduals();
    const double scale = constraints.size() / sum_dist;
    auto scaled_loss = new ::ceres::ScaledLoss(NULL, weight,
			   ::ceres::Ownership::TAKE_OWNERSHIP);
    for (auto const& t : constraints)
    {
      auto cam_itr = std::get<0>(t);
      const double s = scale * std::get<1>(t);
      ::ceres::CostFunction* smoothness_cost  =
	camera_position_smoothness::create(s, std::get<2>(t));
      problem.AddResidualBlock(smoothness_cost,
			       scaled_loss,
			       (cam_itr-1)->second,
			       cam_itr->second,
			       (cam_itr+1)->second);
    }
  }
}

/// Add the camera forward motion damping costs to the Ceres problem
void
camera_options
::add_forward_motion_damping_cost(
    ::ceres::Problem& problem,
    frame_params_t const& ordered_params,
    cam_intrinsic_id_map_t const& frame_to_intr_map) const
{
  if( this->camera_forward_motion_damping > 0.0 &&
      ordered_params.size() >= 2 )
  {
    double sum_dist = 0.0;
    auto prev_cam = ordered_params.begin();
    auto curr_cam = prev_cam;
    curr_cam++;
    std::vector<decltype(curr_cam) > constraints;
    for (; curr_cam != ordered_params.end();
	   prev_cam = curr_cam, curr_cam++)
    {
      // add a forward motion residual only when the camera intrinsic models
      // are not the same instance
      auto prev_idx = frame_to_intr_map.find(prev_cam->first);
      auto curr_idx = frame_to_intr_map.find(curr_cam->first);
      if (prev_idx != frame_to_intr_map.end() &&
	  curr_idx != frame_to_intr_map.end() &&
	  prev_idx->second != curr_idx->second)
      {
	sum_dist += (Eigen::Map<vector_3d>(prev_cam->second + 3) -
		     Eigen::Map<vector_3d>(curr_cam->second + 3)).norm();
	constraints.push_back(curr_cam);
      }
    }
    // Normalize the weight
    const double weight = this->camera_forward_motion_damping
			* problem.NumResiduals();
    const double scale = constraints.size() / sum_dist;
    auto scaled_loss = new ::ceres::ScaledLoss(NULL, weight,
			   ::ceres::Ownership::TAKE_OWNERSHIP);
    for (auto curr_constraint : constraints)
    {
      auto prev_constraint = curr_constraint - 1;
      double inv_dist = 1.0 / static_cast<double>(curr_constraint->first -
						  prev_constraint->first);
      ::ceres::CostFunction* fwd_mo_cost =
	camera_limit_forward_motion::create(scale * inv_dist);
      problem.AddResidualBlock(fwd_mo_cost,
			       scaled_loss,
			       prev_constraint->second,
			       curr_constraint->second);
    }
  }
}

} // end namespace ceres
} // end namespace arrows
} // end namespace kwiver
