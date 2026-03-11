// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of estimate_canonical_transform algorithm

#include "estimate_canonical_transform.h"

#include <vital/logger/logger.h>
#include <vital/types/camera_perspective.h>

#include <algorithm>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace core {

/// Private implementation class
class estimate_canonical_transform::priv
{
public:
  /// Constructor
  priv( estimate_canonical_transform& parent )
    : parent( parent )
  {}

  estimate_canonical_transform& parent;

  // Configuration values
  bool c_estimate_scale() { return parent.c_estimate_scale; }
  double c_height_percentile() { return parent.c_height_percentile; }
};

// ----------------------------------------------------------------------------
void
estimate_canonical_transform
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.core.estimate_canonical_transform" );
}

// Destructor
estimate_canonical_transform
::~estimate_canonical_transform()
{}

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
  // find the centroid and scale of all the landmarks
  typedef vital::landmark_map::map_landmark_t lm_map_t;

  vital::vector_3d center( 0, 0, 0 );
  double s = 0.0;
  vital::matrix_3x3d covar = vital::matrix_3x3d::Zero();
  for( const lm_map_t::value_type& p : landmarks->landmarks() )
  {
    vital::vector_3d pt = p.second->loc();
    center += pt;
    covar += pt * pt.transpose();
    s += pt.dot( pt );
  }

  const double num_lm = static_cast< double >( landmarks->size() );
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

  if( d_->c_height_percentile() >= 0.0 && d_->c_height_percentile() < 1.0 )
  {
    const vital::vector_3d z = rot.col( 2 );
    std::vector< double > heights;
    for( const lm_map_t::value_type& p : landmarks->landmarks() )
    {
      vital::vector_3d pt = p.second->loc();
      heights.push_back( z.dot( pt - center ) );
    }
    std::sort( heights.begin(), heights.end() );

    auto const idx = static_cast< size_t >( d_->c_height_percentile() *
                                            heights.size() );
    center += heights[ idx ] * z;
  }

  if( !d_->c_estimate_scale() )
  {
    s = 1.0;
  }

  vital::rotation_d R( rot );
  R = R.inverse();
  return vital::similarity_d( s, R, R * ( -s * center ) );
}

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
