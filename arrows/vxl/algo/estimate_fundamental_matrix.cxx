// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief vxl estimate fundamental matrix implementation

#include "estimate_fundamental_matrix.h"

#include <vital/types/feature.h>
#include <vital/vital_config.h>

#include <arrows/mvg/epipolar_geometry.h>
#include <arrows/vxl/camera.h>

#include <Eigen/LU>
#include <vgl/vgl_point_2d.h>

#include <vpgl/algo/vpgl_fm_compute_7_point.h>
#include <vpgl/algo/vpgl_fm_compute_8_point.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace vxl {

// ----------------------------------------------------------------------------
/// Private implementation class
class estimate_fundamental_matrix::priv
{
public:
  /// Constructor
  priv( estimate_fundamental_matrix& parent ) : parent( parent ) {}

  estimate_fundamental_matrix& parent;

  bool
  c_precondition() const { return parent.c_precondition; }
  const std::string&
  c_method() const { return parent.c_method; }
};

// ----------------------------------------------------------------------------
void
estimate_fundamental_matrix
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.vxl.estimate_fundamental_matrix" );
}

/// Check that the algorithm's current configuration is valid
bool
estimate_fundamental_matrix
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
/// Estimate an essential matrix from corresponding points
fundamental_matrix_sptr
estimate_fundamental_matrix
::estimate(
  const std::vector< vector_2d >& pts1,
  const std::vector< vector_2d >& pts2,
  std::vector< bool >& inliers,
  double inlier_scale ) const
{
  std::vector< vgl_homg_point_2d< double > > right_points, left_points;
  for( const vector_2d& v : pts1 )
  {
    right_points.push_back( vgl_homg_point_2d< double >( v.x(), v.y() ) );
  }
  for( const vector_2d& v : pts2 )
  {
    left_points.push_back( vgl_homg_point_2d< double >( v.x(), v.y() ) );
  }

  vpgl_fundamental_matrix< double > vfm;
  if( method_converter().from_string( d->c_method() ) == EST_8_POINT )
  {
    vpgl_fm_compute_8_point fm_compute( d->c_precondition() );
    fm_compute.compute( right_points, left_points, vfm );
  }
  else
  {
    std::vector< vpgl_fundamental_matrix< double >* > vfms;
    vpgl_fm_compute_7_point fm_compute( d->c_precondition() );
    fm_compute.compute( right_points, left_points, vfms );
    // TODO use the multiple solutions in a RANSAC framework
    // For now, only keep the first solution
    vfm = *vfms[ 0 ];
    for( auto v : vfms )
    {
      delete v;
    }
  }

  matrix_3x3d F( vfm.get_matrix().data_block() );
  F.transposeInPlace();

  fundamental_matrix_sptr fm( new fundamental_matrix_d( F ) );
  inliers = mvg::mark_fm_inliers( *fm, pts1, pts2, inlier_scale );
  return fm;
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
