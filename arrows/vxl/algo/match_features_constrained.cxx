// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VXL match_features_constrained algorithm implementation

#include "match_features_constrained.h"

#include <vector>

#include <vital/types/descriptor_set.h>
#include <vital/types/feature_set.h>
#include <vital/types/match_set.h>

#include <rsdl/rsdl_kd_tree.h>
#include <vnl/vnl_vector_fixed.h>

#include <limits>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace vxl {

// Private implementation class
class match_features_constrained::priv
{
public:
  // Constructor
  priv( match_features_constrained& parent ) : parent( parent ) {}

  match_features_constrained& parent;

  // --------------------------------------------------------------------------
  // Compute the minimum angle between two angles in degrees
  inline static
  double
  angle_dist( double a1, double a2 )
  {
    double d = a1 - a2;
    if( d > 180.0 )
    {
      d -= 360;
    }
    if( d < -180.0 )
    {
      d += 360;
    }
    return fabs( d );
  }

  void
  match(
    feature_set_sptr feat1, descriptor_set_sptr desc1,
    feature_set_sptr feat2, descriptor_set_sptr desc2,
    std::vector< vital::match >& matches ) const
  {
    matches.clear();

    std::vector< rsdl_point > fixedpts;
    const std::vector< feature_sptr >& feat1_vec = feat1->features();
    const std::vector< feature_sptr >& feat2_vec = feat2->features();
    const std::vector< descriptor_sptr > desc1_vec( desc1->cbegin(),
      desc1->cend() );
    const std::vector< descriptor_sptr > desc2_vec( desc2->cbegin(),
      desc2->cend() );

    for( unsigned int i = 0; i < feat2_vec.size(); i++ )
    {
      rsdl_point pt( 2 );
      pt.set_cartesian(
        vnl_vector_fixed< double,
          2 >( feat2_vec[ i ]->loc().data() ) );
      fixedpts.push_back( pt );
    }

    rsdl_kd_tree kdtree( fixedpts );

    for( unsigned int i = 0; i < feat1_vec.size(); i++ )
    {
      vital::feature_sptr f1 = feat1_vec[ i ];

      std::vector< rsdl_point > points;
      std::vector< int > indices;
      rsdl_point query_pt( 2 );

      query_pt.set_cartesian(
        vnl_vector_fixed< double,
          2 >( f1->loc().data() ) );
      kdtree.points_in_radius(
        query_pt, this->c_radius_thresh(), points,
        indices );

      int closest = -1;
      double closest_dist = std::numeric_limits< double >::max();
      vnl_vector< double > d1( desc1_vec[ i ]->as_double().data(),
        static_cast< unsigned >( desc1_vec[ i ]->size() ) );

      for( unsigned int j = 0; j < indices.size(); j++ )
      {
        int index = indices[ j ];
        vital::feature_sptr f2 = feat2_vec[ index ];
        if( ( c_scale_thresh() <= 0.0  || std::max(
          f1->scale(),
          f2->scale() ) / std::min(
                f1->scale(),
                f2->scale() ) <= c_scale_thresh() ) &&
            ( c_angle_thresh() <= 0.0  || angle_dist(
              f2->angle(),
              f1->angle() ) <= c_angle_thresh() ) )
        {
          vnl_vector< double > d2( desc2_vec[ index ]->as_double().data(),
            static_cast< unsigned >( desc2_vec[ index ]->size() ) );
          double dist = ( d1 - d2 ).squared_magnitude();
          if( dist < closest_dist )
          {
            closest = index;
            closest_dist = dist;
          }
        }
      }

      if( closest >= 0 )
      {
        matches.push_back( vital::match( i, closest ) );
      }
    }

    LOG_INFO( m_logger, "Found " << matches.size() << " matches." );
  }

  double
  c_scale_thresh() const { return parent.c_scale_thresh; }
  double
  c_angle_thresh() const { return parent.c_angle_thresh; }
  double
  c_radius_thresh() const { return parent.c_radius_thresh; }

  vital::logger_handle_t m_logger;
};

// ----------------------------------------------------------------------------
void
match_features_constrained
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.vxl.match_features_constrained" );
}

// ----------------------------------------------------------------------------
// Check that the algorithm's configuration vital::config_block is valid
bool
match_features_constrained
::check_configuration( vital::config_block_sptr config ) const
{
  double radius_thresh = config->get_value< double >(
    "radius_thresh",
    d->c_radius_thresh() );
  if( radius_thresh <= 0.0 )
  {
    LOG_ERROR(
      logger(),
      "radius_thresh should be > 0.0, is " << radius_thresh );
    return false;
  }

  double scale_thresh = config->get_value< double >(
    "scale_thresh",
    d->c_scale_thresh() );
  if( scale_thresh < 1.0 && scale_thresh >= 0.0 )
  {
    LOG_ERROR(
      logger(), "scale_thresh should be >= 1.0 (or < 0.0 to disable), is "
        << scale_thresh );
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
// Match one set of features and corresponding descriptors to another
vital::match_set_sptr
match_features_constrained
::match(
  vital::feature_set_sptr feat1, vital::descriptor_set_sptr desc1,
  vital::feature_set_sptr feat2, vital::descriptor_set_sptr desc2 ) const
{
  if( !feat1 || !feat2 || !desc1 || !desc2 )
  {
    return match_set_sptr();
  }

  std::vector< vital::match > matches;
  d->match( feat1, desc1, feat2, desc2, matches );

  return std::make_shared< vital::simple_match_set >(
    vital::simple_match_set(
      matches ) );
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
