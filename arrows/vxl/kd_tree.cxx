// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation of detect_feature_filtered algorithm
 */
#include "kd_tree.h"
#include <vital/algo/nearest_neighbors.h>

#include <rsdl/rsdl_dist.h>
#include <rsdl/rsdl_kd_tree.h>
#include <vnl/vnl_vector_fixed.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace vxl {

/// Private implementation class
class kd_tree::priv
{
public:
  /// Constructor
  priv() {}

  vital::logger_handle_t m_logger;

  std::unique_ptr< rsdl_kd_tree > m_kd_tree;
};

// ----------------------------------------------------------------------------
// Constructor
kd_tree
::kd_tree()
  : d_( new priv )
{
  attach_logger( "arrows.vxl.kd_tree" );
  d_->m_logger = logger();
}

// Destructor
kd_tree
::~kd_tree()
{
}

// ------------------------------------------------------------------
vital::config_block_sptr
kd_tree
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config =
    vital::algo::nearest_neighbors::get_configuration();

  return config;
}

// ------------------------------------------------------------------
void
kd_tree
::set_configuration( vital::config_block_sptr in_config )
{
}

// ------------------------------------------------------------------
bool
kd_tree
::check_configuration( vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
// Build the search tree
void
kd_tree
::build( std::vector< vital::point_3d >& points ) const
{
  std::vector< rsdl_point > vxl_points;

  for( auto p : points )
  {
    rsdl_point pt( 3 );
    pt.set_cartesian( vnl_vector_fixed< double, 3 >( p.value().data() ) );
    vxl_points.push_back( pt );
  }

  d_->m_kd_tree.reset( new rsdl_kd_tree( vxl_points ) );
}

// ----------------------------------------------------------------------------
// Return the K nearest points for a target point
void
kd_tree
::find_nearest_point(
  vital::point_3d& point,
  int K,
  std::vector< int >& indices,
  std::vector< double >& distances ) const
{
  if( d_->m_kd_tree )
  {
    rsdl_point pt( 3 );
    std::vector< rsdl_point > closest_pts;
    pt.set_cartesian( vnl_vector_fixed< double, 3 >( point.value().data() ) );
    d_->m_kd_tree->n_nearest( pt, K, closest_pts, indices );

    // Calculate distances
    for( auto cp : closest_pts )
    {
      distances.push_back( rsdl_dist( pt, cp ) );
    }
  }
  else
  {
    LOG_ERROR( logger(), "The search tree must be built first." );
  }
}

// ----------------------------------------------------------------------------
// Return the K nearest points for multiple target points
void
kd_tree
::find_nearest_points(
  std::vector< point_3d >& vec,
  int K,
  std::vector< std::vector< int > >& indices,
  std::vector< std::vector< double > >& distances ) const
{
  if( d_->m_kd_tree )
  {
    for( auto p : vec )
    {
      std::vector< int > p_indices;
      std::vector< double > p_distances;
      this->find_nearest_point( p, K, p_indices, p_distances );

      indices.push_back( p_indices );
      distances.push_back( p_distances );
    }
  }
  else
  {
    LOG_ERROR( logger(), "The search tree must be built first." );
  }
}

// ----------------------------------------------------------------------------
// Return the nearest points within a radius of a target point
void
kd_tree
::find_within_radius(
  vital::point_3d& point,
  double r,
  std::vector< int >& indices ) const
{
  if( d_->m_kd_tree )
  {
    rsdl_point pt( 3 );
    std::vector< rsdl_point > closest_pts;
    pt.set_cartesian( vnl_vector_fixed< double, 3 >( point.value().data() ) );

    d_->m_kd_tree->points_in_radius( pt, r, closest_pts, indices );
  }
  else
  {
    LOG_ERROR( logger(), "The search tree must be built first." );
  }
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
