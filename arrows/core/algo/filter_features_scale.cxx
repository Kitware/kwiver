// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of filter_features_scale algorithm
#include "filter_features_scale.h"

#include <algorithm>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace core {

// Helper struct for the filter function
struct feature_at_index_is_greater
{
  bool
  operator()(
    std::pair< size_t, double > const& l,
    std::pair< size_t, double > const& r )
  {
    return l.second > r.second;
  }
};

// Private implementation class
class filter_features_scale::priv
{
public:
  priv( filter_features_scale& parent )
    : parent( parent )
  {}

  filter_features_scale& parent;

  // Configuration Parameters for access outside this class
  double
  c_top_fraction() const { return parent.c_top_fraction; }
  size_t
  c_min_features() const { return parent.c_min_features; }
  size_t
  c_max_features() const { return parent.c_max_features; }

// ----------------------------------------------------------------------------
  feature_set_sptr
  filter( feature_set_sptr feat, std::vector< size_t >& ind ) const
  {
    const std::vector< feature_sptr >& feat_vec = feat->features();
    ind.clear();
    if( feat_vec.size() <= parent.c_min_features )
    {
      ind.resize( feat_vec.size() );
      for( size_t i = 0; i < ind.size(); ++i )
      {
        ind[ i ] = i;
      }
      return feat;
    }

    //  Create a new vector with the index and scale for faster sorting
    std::vector< std::pair< size_t, double > > indices;
    indices.reserve( feat_vec.size() );
    for( size_t i = 0; i < feat_vec.size(); i++ )
    {
      indices.push_back( std::make_pair( i, feat_vec[ i ]->scale() ) );
    }

    // compute threshold
    auto cutoff = std::max(
      parent.c_min_features,
      static_cast< size_t >( parent.c_top_fraction * indices.size() ) );
    if( parent.c_max_features > 0 )
    {
      cutoff = std::min( cutoff, parent.c_max_features );
    }

    // partially sort on descending feature scale
    std::nth_element(
      indices.begin(), indices.begin() + cutoff, indices.end(),
      feature_at_index_is_greater() );

    std::vector< feature_sptr > filtered( cutoff );
    ind.resize( cutoff );
    for( size_t i = 0; i < cutoff; i++ )
    {
      auto const index = indices[ i ].first;
      ind[ i ] = index;
      filtered[ i ] = feat_vec[ index ];
    }

    LOG_INFO(
      parent.logger(),
      "Reduced " << feat_vec.size() << " features to " <<
        filtered.size() << " features." );

    return std::make_shared< vital::simple_feature_set >(
      vital::simple_feature_set( filtered ) );
  }
};

// ----------------------------------------------------------------------------
void
filter_features_scale
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.core.filter_features_scale" );
}

// Destructor
filter_features_scale
::~filter_features_scale()
{}

// Check that the algorithm's configuration vital::config_block is valid
bool
filter_features_scale
::check_configuration( vital::config_block_sptr config ) const
{
  double top_fraction = config->get_value< double >(
    "top_fraction",
    d_->c_top_fraction() );
  if( top_fraction <= 0.0 || top_fraction > 1.0 )
  {
    LOG_ERROR(
      logger(),
      "top_fraction parameter is " << top_fraction <<
        ", needs to be in (0.0, 1.0]." );
    return false;
  }

  auto const min_features =
    config->get_value< size_t >( "min_features", d_->c_min_features() );
  auto const max_features =
    config->get_value< size_t >( "max_features", d_->c_max_features() );
  if( max_features > 0 && max_features < min_features )
  {
    LOG_ERROR(
      logger(), "max_features (" << max_features
                                 <<
        ") must be zero or greater than min_features ("
                                 << min_features << ")" );
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// Filter feature set
vital::feature_set_sptr
filter_features_scale
::filter(
  vital::feature_set_sptr feat,
  std::vector< size_t >& indices ) const
{
  return d_->filter( feat, indices );
}

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
