// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining CERES algorithm implementation of camera
/// optimization.

#include "optimize_cameras.h"
#include <arrows/ceres/logging.h>
#include <arrows/ceres/options.h>
#include <arrows/ceres/reprojection_error.h>
#include <vital/exceptions.h>
#include <vital/vital_config.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ceres {

// Private implementation class
class optimize_cameras::priv
{
public:
  priv( optimize_cameras& parent )
    : parent( parent )
  {}

  optimize_cameras& parent;

  bool
  c_verbose() const { return parent.c_verbose; }
  LossFunctionType
  c_loss_function_type() const { return parent.c_loss_function_type; }
  double
  c_loss_function_scale() const { return parent.c_loss_function_scale; }
};

// ----------------------------------------------------------------------------
void
optimize_cameras
::initialize()
{
  init_logging();

  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.ceres.optimize_cameras" );
}

// ----------------------------------------------------------------------------
// Check that the algorithm's currently configuration is valid
bool
optimize_cameras
::check_configuration( VITAL_UNUSED config_block_sptr config ) const
{
  std::string msg;
  if( !c_solver_options->options.IsValid( &msg ) )
  {
    LOG_ERROR( logger(), msg);
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// Optimize camera parameters given sets of landmarks and feature tracks
void
optimize_cameras
::optimize(
  vital::camera_map_sptr& cameras,
  vital::feature_track_set_sptr tracks,
  vital::landmark_map_sptr landmarks,
  vital::sfm_constraints_sptr constraints ) const
{
  if( !cameras || !landmarks || !tracks )
  {
    VITAL_THROW(
      vital::invalid_value,
      "One or more input data pieces are Null!" );
  }
  typedef camera_map::map_camera_t map_camera_t;
  typedef landmark_map::map_landmark_t map_landmark_t;

  // extract data from containers
  map_camera_t cams = cameras->cameras();
  map_landmark_t lms = landmarks->landmarks();
  std::vector< track_sptr > trks = tracks->tracks();

  // Extract the landmark locations into a mutable map
  typedef std::map< track_id_t, std::vector< double > > lm_param_map_t;

  lm_param_map_t landmark_params;
  for( const map_landmark_t::value_type& lm : lms )
  {
    vector_3d loc = lm.second->loc();
    landmark_params[ lm.first ] = std::vector< double >(
      loc.data(),
      loc.data() + 3 );
  }

  // a map from frame number to extrinsic parameters
  typedef std::unordered_map< frame_id_t,
    std::vector< double > > cam_param_map_t;

  cam_param_map_t camera_params;
  // vector of unique camera intrinsic parameters
  std::vector< std::vector< double > > camera_intr_params;
  // a map from frame number to index of unique camera intrinsics in
  // camera_intr_params
  std::unordered_map< frame_id_t, size_t > frame_to_intr_map;

  // Extract the raw camera parameter into the provided maps
  c_camera_options->extract_camera_parameters(
    cams,
    camera_params,
    camera_intr_params,
    frame_to_intr_map );

  // the Ceres solver problem
  ::ceres::Problem problem;

  // enumerate the intrinsics held constant
  std::vector< int > constant_intrinsics =
    c_camera_options->enumerate_constant_intrinsics();

  // Create the loss function to use
  ::ceres::LossFunction* loss_func =
    LossFunctionFactory(
      c_loss_function_type,
      c_loss_function_scale );
  bool loss_func_used = false;

  // Add the residuals for each relevant observation
  for( const track_sptr& t : trks )
  {
    const track_id_t id = t->id();
    lm_param_map_t::iterator lm_itr = landmark_params.find( id );
    // skip this track if the landmark is not in the set to optimize
    if( lm_itr == landmark_params.end() )
    {
      continue;
    }

    for( track::history_const_itr ts = t->begin(); ts != t->end(); ++ts )
    {
      cam_param_map_t::iterator cam_itr =
        camera_params.find( ( *ts )->frame() );
      if( cam_itr == camera_params.end() )
      {
        continue;
      }

      auto const intr_idx = frame_to_intr_map[ ( *ts )->frame() ];
      double* intr_params_ptr = &camera_intr_params[ intr_idx ][ 0 ];
      auto fts = std::dynamic_pointer_cast< feature_track_state >( *ts );
      if( !fts || !fts->feature )
      {
        continue;
      }

      vector_2d pt = fts->feature->loc();
      problem.AddResidualBlock(
        create_cost_func(
          c_camera_options->lens_distortion_type,
          pt.x(), pt.y() ),
        loss_func,
        intr_params_ptr,
        &cam_itr->second[ 0 ],
        &lm_itr->second[ 0 ] );
      loss_func_used = true;
    }
  }

  auto const ndp =
    num_distortion_params( c_camera_options->lens_distortion_type );
  for( std::vector< double >& cip : camera_intr_params )
  {
    // apply the constraints
    if( constant_intrinsics.size() > 4 + ndp )
    {
      // set all parameters in the block constant
      problem.SetParameterBlockConstant( &cip[ 0 ] );
    }
    else if( !constant_intrinsics.empty() )
    {
      // set a subset of parameters in the block constant
#if CERES_VERSION_MAJOR >= 2
      problem.SetManifold(
        &cip[ 0 ],
        new ::ceres::SubsetManifold( 5 + ndp, constant_intrinsics ) );
#else
      problem.SetParameterization(
        &cip[ 0 ],
        new ::ceres::SubsetParameterization( 5 + ndp, constant_intrinsics ) );
#endif
    }
  }
  // Set the landmarks constant
  for( lm_param_map_t::value_type& lmp : landmark_params )
  {
    problem.SetParameterBlockConstant( &lmp.second[ 0 ] );
  }

  if( c_camera_options->camera_path_smoothness > 0.0 ||
      c_camera_options->camera_forward_motion_damping > 0.0 )
  {
    // sort the camera parameters in order of frame number
    std::vector< std::pair< vital::frame_id_t, double* > > ordered_params;
    for( auto& item : camera_params )
    {
      ordered_params.push_back(
        std::make_pair(
          item.first,
          &item.second[ 0 ] ) );
    }
    std::sort( ordered_params.begin(), ordered_params.end() );

    // Add camera path regularization residuals
    c_camera_options->add_camera_path_smoothness_cost(
      problem,
      ordered_params );

    // Add forward motion regularization residuals
    c_camera_options->add_forward_motion_damping_cost(
      problem, ordered_params,
      frame_to_intr_map );
  }

  // add costs for priors
  c_camera_options->add_position_prior_cost(
    problem, camera_params,
    constraints );

  c_camera_options->add_intrinsic_priors_cost( problem, camera_intr_params );

  // If the loss function was added to a residual block, ownership was
  // transfered.  If not then we need to delete it.
  if( loss_func && !loss_func_used )
  {
    delete loss_func;
  }

  ::ceres::Solver::Summary summary;
  ::ceres::Solve( c_solver_options->options, &problem, &summary );
  if( c_verbose )
  {
    LOG_DEBUG( logger(), "Ceres Full Report:\n" << summary.FullReport() );
  }

  // Update the cameras with the optimized values
  c_camera_options->update_camera_parameters(
    cams, camera_params,
    camera_intr_params, frame_to_intr_map );
  cameras = std::make_shared< simple_camera_map >( cams );
}

// ----------------------------------------------------------------------------
// Optimize a single camera given corresponding features and landmarks
void
optimize_cameras
::optimize(
  vital::camera_perspective_sptr& camera,
  const std::vector< vital::feature_sptr >& features,
  const std::vector< vital::landmark_sptr >& landmarks,
  VITAL_UNUSED kwiver::vital::sfm_constraints_sptr constraints ) const
{
  // extract camera parameters to optimize
  auto const ndp =
    num_distortion_params( c_camera_options->lens_distortion_type );
  std::vector< double > cam_intrinsic_params( 5 + ndp, 0.0 );
  std::vector< double > cam_extrinsic_params( 6 );
  c_camera_options->extract_camera_extrinsics(
    camera,
    &cam_extrinsic_params[ 0 ] );

  camera_intrinsics_sptr K = camera->intrinsics();
  c_camera_options->extract_camera_intrinsics( K, &cam_intrinsic_params[ 0 ] );

  // extract the landmark parameters
  std::vector< std::vector< double > > landmark_params;
  for( auto const& lm : landmarks )
  {
    vector_3d loc = lm->loc();
    landmark_params.push_back(
      std::vector< double >(
        loc.data(),
        loc.data() + 3 ) );
  }

  // the Ceres solver problem
  ::ceres::Problem problem;

  // enumerate the intrinsics held constant
  std::vector< int > constant_intrinsics =
    c_camera_options->enumerate_constant_intrinsics();

  // Create the loss function to use
  ::ceres::LossFunction* loss_func =
    LossFunctionFactory(
      c_loss_function_type,
      c_loss_function_scale );

  // Add the residuals for each relevant observation
  for( size_t i = 0; i < features.size(); ++i )
  {
    vector_2d pt = features[ i ]->loc();
    problem.AddResidualBlock(
      create_cost_func(
        c_camera_options->lens_distortion_type,
        pt.x(), pt.y() ),
      loss_func,
      &cam_intrinsic_params[ 0 ],
      &cam_extrinsic_params[ 0 ],
      &landmark_params[ i ][ 0 ] );

    problem.SetParameterBlockConstant( &landmark_params[ i ][ 0 ] );
  }

  // set contraints on the camera intrinsics
  if( constant_intrinsics.size() > 4 + ndp )
  {
    // set all parameters in the block constant
    problem.SetParameterBlockConstant( &cam_intrinsic_params[ 0 ] );
  }
  else if( !constant_intrinsics.empty() )
  {
    // set a subset of parameters in the block constant
#if CERES_VERSION_MAJOR >= 2
    problem.SetManifold(
      &cam_intrinsic_params[ 0 ],
      new ::ceres::SubsetManifold( 5 + ndp, constant_intrinsics ) );
#else
    problem.SetParameterization(
      &cam_intrinsic_params[ 0 ],
      new ::ceres::SubsetParameterization( 5 + ndp, constant_intrinsics ) );
#endif
  }

  // If the loss function was added to a residual block, ownership was
  // transfered.  If not then we need to delete it.
  if( loss_func && !features.empty() )
  {
    delete loss_func;
  }

  ::ceres::Solver::Summary summary;
  ::ceres::Solve( c_solver_options->options, &problem, &summary );
  if( c_verbose )
  {
    LOG_DEBUG( logger(), "Ceres Full Report:\n" << summary.FullReport() );
  }

  // update the cameras from optimized parameters
  // only create a new intrinsics object if the values were not held constant
  if( c_camera_options->optimize_intrinsics() )
  {
    auto new_K = std::make_shared< simple_camera_intrinsics >();
    c_camera_options->update_camera_intrinsics(
      new_K,
      &cam_intrinsic_params[ 0 ] );
    K = new_K;
  }

  auto new_camera = std::make_shared< simple_camera_perspective >();
  new_camera->set_intrinsics( K );
  c_camera_options->update_camera_extrinsics(
    new_camera,
    &cam_extrinsic_params[ 0 ] );
  camera = new_camera;
}

} // end namespace ceres

} // end namespace arrows

} // end namespace kwiver
