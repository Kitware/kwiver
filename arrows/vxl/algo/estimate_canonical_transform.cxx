// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of VXL estimate_canonical_transform algorithm

#include "estimate_canonical_transform.h"

#include <vital/logger/logger.h>
#include <vital/types/camera_perspective.h>
#include <vital/vital_config.h>

#include <algorithm>

#include <rrel/rrel_irls.h>
#include <rrel/rrel_lms_obj.h>
#include <rrel/rrel_orthogonal_regression.h>
#include <rrel/rrel_ran_sam_search.h>
#include <rrel/rrel_ransac_obj.h>
#include <rrel/rrel_tukey_obj.h>
#include <vnl/vnl_double_3.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace vxl {

// Private implementation class
class estimate_canonical_transform::priv
{
public:
// Constructor
  priv( estimate_canonical_transform& parent ) : parent( parent ) {}

  estimate_canonical_transform& parent;

// --------------------------------------------------------------------------
// Helper function to estimate a ground plane from the data points
  vector_4d
  estimate_plane( std::vector< vector_3d > const& points ) const
  {
    std::vector< vnl_vector< double > > vnl_points;
    for( vector_3d const& p : points )
    {
      vnl_points.push_back(
        vnl_vector< double >(
          vnl_double_3(
            p[ 0 ], p[ 1 ],
            p[ 2 ] ) ) );
    }

    // The number of different populations in the data set. For most
    // problems, the data is from one source (surface, etc.), so this
    // will be 1.
    int max_pops = 1;

    rrel_orthogonal_regression* reg =
      new rrel_orthogonal_regression( vnl_points );
    vnl_vector< double > pp;

    switch( rrel_converter().from_string( c_rrel_method() ) )
    {
      case RANSAC:
      {
        rrel_ransac_obj* ransac = new rrel_ransac_obj();
        rrel_ran_sam_search* ransam = new rrel_ran_sam_search;
        ransam->set_sampling_params(
          c_max_outlier_frac(), c_desiredprob_good(),
          max_pops );
        ransam->set_trace_level( c_trace_level() );

        reg->set_prior_scale( c_prior_inlier_scale() );

        if( !ransam->estimate( reg, ransac ) )
        {
          LOG_ERROR(
            m_logger,
            "RANSAC unable to fit a plane to the landmarks." );
        }
        LOG_DEBUG( m_logger, "Estimated scale = " << ransam->scale() );
        pp = ransam->params();
        delete ransam;
        delete ransac;
      }
      case LMS:
      {
        int num_sam_inst = reg->num_samples_to_instantiate();
        rrel_objective* lms = new rrel_lms_obj( num_sam_inst );
        rrel_ran_sam_search* ransam = new rrel_ran_sam_search;
        ransam->set_sampling_params(
          c_max_outlier_frac(), c_desiredprob_good(),
          max_pops );
        ransam->set_trace_level( c_trace_level() );

        if( !ransam->estimate( reg, lms ) )
        {
          LOG_ERROR(m_logger, "LMS unable to fit a plane to the landmarks." );
        }
        LOG_DEBUG( m_logger, "Estimated scale = " << ransam->scale() );
        pp = ransam->params();
        delete ransam;
        delete lms;
      }
      case IRLS:
      {
        //  Beaton-Tukey loss function
        rrel_m_est_obj* m_est = new rrel_tukey_obj( 4.0 );

        reg->set_no_prior_scale();

        // Iteratively Reweighted Least Squares
        rrel_irls* irls = new rrel_irls( c_irls_max_iterations() );
        irls->set_est_scale( c_irls_iterations_for_scale() );
        irls->set_convergence_test( c_irls_conv_tolerance() );
        irls->set_trace_level( c_trace_level() );

        if( !irls->estimate( reg, m_est ) )
        {
          LOG_ERROR(m_logger, "IRLS unable to fit a plane to the landmarks." );
        }
        LOG_DEBUG( m_logger, "Estimated scale = " << irls->scale() );
        pp = irls->params();
        delete irls;
        delete m_est;
      }
    }

    delete reg;
    return vector_4d( pp[ 0 ], pp[ 1 ], pp[ 2 ], pp[ 3 ] );
  }

// Enable estimation of scale in the similarity transform
  bool
  c_estimate_scale() const { return parent.c_estimate_scale; }
// This controls the verbosity of the search techniques.
  int
  c_trace_level() const { return parent.c_trace_level; }
// The robust estimation method to used
  const std::string&
  c_rrel_method() const { return parent.c_rrel_method; }
// The desired probability of finding the correct fit.
  double
  c_desiredprob_good() const { return parent.c_estimate_scale; }
// The maximum fraction of the data that is expected to be gross outliers.
  double
  c_max_outlier_frac() const { return parent.c_max_outlier_frac; }
// The initial estimate of inlier scale for RANSAC
  double
  c_prior_inlier_scale() const { return parent.c_prior_inlier_scale; }
// The maximum number of iterations for IRLS
  int
  c_irls_max_iterations() const { return parent.c_irls_max_iterations; }

// The number of IRLS iterations in which to estimate scale
  int
  c_irls_iterations_for_scale() const
  {
    return parent.c_irls_iterations_for_scale;
  }

// The convergence tolerance for IRLS
  double
  c_irls_conv_tolerance() const { return parent.c_irls_conv_tolerance; }

// Logger handle
  vital::logger_handle_t m_logger;
};

// ----------------------------------------------------------------------------
void
estimate_canonical_transform
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.vxl.estimate_canonical_transform" );
}

// ----------------------------------------------------------------------------
// Check that the algorithm's configuration vital::config_block is valid
bool
estimate_canonical_transform
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
// Estimate a canonical similarity transform for cameras and points
kwiver::vital::similarity_d
estimate_canonical_transform
::estimate_transform(
  kwiver::vital::camera_map_sptr const cameras,
  kwiver::vital::landmark_map_sptr const landmarks ) const
{
  using namespace arrows;

  std::vector< vector_3d > points;
  for( auto const& p : landmarks->landmarks() )
  {
    points.push_back( p.second->loc() );
  }

  // estimate the ground plane
  vector_4d plane = d->estimate_plane( points );
  vector_3d normal = plane.head< 3 >();

  // project the points onto the plane
  for( vector_3d& p : points )
  {
    p -= ( normal.dot( p ) + plane[ 3 ] ) * normal;
  }

  // find the centroid and scale of all the landmarks
  vital::vector_3d center( 0, 0, 0 );
  double s = 0.0;
  vital::matrix_3x3d covar = vital::matrix_3x3d::Zero();
  for( vector_3d const& p : points )
  {
    center += p;
    covar += p * p.transpose();
    s += p.dot( p );
  }

  const double num_lm = static_cast< double >( points.size() );
  center /= num_lm;
  covar /= num_lm;
  covar -= center * center.transpose();
  s /= num_lm;
  s -= center.dot( center );
  s = 1.0 / std::sqrt( s );

  Eigen::JacobiSVD< vital::matrix_3x3d > svd( covar, Eigen::ComputeFullV );
  vital::matrix_3x3d rot = svd.matrixV();
  // ensure that rot is a rotation (determinant 1)
  rot.col( 1 ) = rot.col( 2 ).cross( rot.col( 0 ) ).normalized();

  if( cameras->size() > 0 )
  {
    // find the average camera center and  average up direction
    vital::vector_3d cam_center( 0, 0, 0 );
    vital::vector_3d cam_up( 0, 0, 0 );
    typedef vital::camera_map::map_camera_t cam_map_t;
    for( const cam_map_t::value_type& p : cameras->cameras() )
    {
      auto cam_ptr =
        std::dynamic_pointer_cast< camera_perspective >( p.second );
      cam_center += cam_ptr->center();
    }
    cam_center /= static_cast< double >( cameras->size() );
    cam_center -= center;
    cam_center = cam_center.normalized();
    // flip the plane normal if it points away from the cameras
    if( cam_center.dot( rot.col( 2 ) ) < 0.0 )
    {
      // rotate 180 about the X-axis
      rot.col( 2 ) = -rot.col( 2 );
      rot.col( 1 ) = -rot.col( 1 );
    }
  }

  if( !d->c_estimate_scale() )
  {
    s = 1.0;
  }

  vital::rotation_d R( rot );
  R = R.inverse();
  return vital::similarity_d( s, R, R * ( -s * center ) );
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
