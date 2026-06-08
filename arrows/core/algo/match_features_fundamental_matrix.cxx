// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of the core match_features_fundamental_matrix
/// algorithm

#include "match_features_fundamental_matrix.h"

#include <algorithm>
#include <iostream>

#include <vital/exceptions/algorithm.h>
#include <vital/types/fundamental_matrix.h>
#include <vital/types/match_set.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace core {

// Private implementation class
class match_features_fundamental_matrix::priv
{
public:
  // Constructor
  priv( match_features_fundamental_matrix& parent )
    : parent( parent )

  {}

  match_features_fundamental_matrix& parent;

  // Configuration values
  double c_inlier_scale() { return parent.c_inlier_scale; }
  int c_min_required_inlier_count()
  { return parent.c_min_required_inlier_count; }
  double c_min_required_inlier_percent()
  { return parent.c_min_required_inlier_percent; }
  double c_motion_filter_percentile()
  { return parent.c_motion_filter_percentile; }

  // processing classes
  vital::algo::match_features_sptr
  c_matcher()
  {
    return parent.c_feature_matcher;
  }

  vital::algo::estimate_fundamental_matrix_sptr c_f_estimator()
  { return parent.c_fundamental_matrix_estimator; }
};

// ----------------------------------------------------------------------------
void
match_features_fundamental_matrix
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.core.match_features_fundamental_matrix" );
}

// Destructor
match_features_fundamental_matrix
::~match_features_fundamental_matrix()
{}

// ----------------------------------------------------------------------------
bool
match_features_fundamental_matrix
::check_configuration( vital::config_block_sptr config ) const
{
  bool config_valid = true;
  double motion_filter_percentile =
    config->get_value< double >(
      "motion_filter_percentile",
      d_->c_motion_filter_percentile() );
  // this algorithm is optional
  if( motion_filter_percentile < 0.0 || motion_filter_percentile > 1.0 )
  {
    config_valid = false;
  }

  return (
    check_nested_algo_configuration
    < vital::algo::estimate_fundamental_matrix >(
      "fundamental_matrix_estimator", config )
    &&
    check_nested_algo_configuration
    < vital::algo::match_features >( "feature_matcher", config )
    &&
    config_valid
  );
}

namespace {

// compute the p-th percentile of the data
double
percentile( std::vector< double > const& data, double p )
{
  p = std::max( 0.0, std::min( 1.0, p ) );

  std::vector< double > d( data );
  size_t nth_idx = static_cast< size_t >( d.size() * p );
  std::nth_element( d.begin(), d.begin() + nth_idx, d.end() );
  return d[ nth_idx ];
}

} // namespace

// ----------------------------------------------------------------------------
// Match one set of features and corresponding descriptors to another
match_set_sptr
match_features_fundamental_matrix
::match(
  feature_set_sptr feat1, descriptor_set_sptr desc1,
  feature_set_sptr feat2, descriptor_set_sptr desc2 ) const
{
  if( !d_->c_matcher() || !d_->c_f_estimator() )
  {
    return match_set_sptr();
  }

  // compute the initial matches
  match_set_sptr init_matches = d_->c_matcher()->match(
    feat1, desc1, feat2,
    desc2 );

  // estimate a fundamental_matrix from the initial matches
  std::vector< bool > inliers;
  fundamental_matrix_sptr F = d_->c_f_estimator()->estimate(
    feat1, feat2, init_matches,
    inliers, d_->c_inlier_scale() );
  int inlier_count = static_cast< int >( std::count(
    inliers.begin(),
    inliers.end(), true ) );
  LOG_INFO(
    logger(),
    "inlier ratio: " << inlier_count << "/" << inliers.size() );

  // verify matching criteria are met
  if( !inlier_count || inlier_count < d_->c_min_required_inlier_count() ||
      static_cast< double >( inlier_count ) / inliers.size() <
      d_->c_min_required_inlier_percent() )
  {
    return match_set_sptr( new simple_match_set() );
  }

  // return the subset of inlier matches
  std::vector< vital::match > m = init_matches->matches();
  std::vector< vital::match > inlier_m;
  for( size_t i = 0; i < inliers.size(); ++i )
  {
    if( inliers[ i ] )
    {
      inlier_m.push_back( m[ i ] );
    }
  }

  if( d_->c_motion_filter_percentile() >= 1.0 )
  {
    return match_set_sptr( new simple_match_set( inlier_m ) );
  }

  // Further filter the matches by motion amount to remove outliers.
  // For relatively small motions there may be outliers that agree
  // with the epipolar geometry but have unusually large motion.
  // Discard matches with motion above the twice the Nth percentile.
  auto f1 = feat1->features();
  auto f2 = feat2->features();
  std::vector< double > dists;
  dists.reserve( inlier_m.size() );
  for( auto const& i_m : inlier_m )
  {
    vector_2d dist = f1[ i_m.first ]->loc() - f2[ i_m.second ]->loc();
    dists.push_back( dist.norm() );
  }

  double max_dist = 2.0 * percentile( dists, d_->c_motion_filter_percentile() );
  std::vector< vital::match > filtered_m;
  for( size_t i = 0; i < inlier_m.size(); ++i )
  {
    if( dists[ i ] < max_dist )
    {
      filtered_m.push_back( inlier_m[ i ] );
    }
  }

  LOG_DEBUG(
    logger(), "Filtered " << inlier_m.size() - filtered_m.size() <<
      " matches with motion greater than " << max_dist);

  return match_set_sptr( new simple_match_set( filtered_m ) );
}

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
