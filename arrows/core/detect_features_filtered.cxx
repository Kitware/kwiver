// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of detect_feature_filtered algorithm
#include "detect_features_filtered.h"
#include <vital/algo/filter_features.h>

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace core {

/// Private implementation class
class detect_features_filtered::priv
{
public:
  /// Constructor
  priv()
    : feature_detector(nullptr),
      feature_filter(nullptr)
  {
  }

  /// The feature detector algorithm to use
  vital::algo::detect_features_sptr feature_detector;

  /// The feature filter algorithm to use
  vital::algo::filter_features_sptr feature_filter;

  vital::logger_handle_t m_logger;
};

// ----------------------------------------------------------------------------
// Constructor
detect_features_filtered
::detect_features_filtered()
: d_(new priv)
{
  attach_logger( "arrows.core.detect_features_filtered" );
  d_->m_logger = logger();
}

// Destructor
detect_features_filtered
::~detect_features_filtered()
{
}

// ----------------------------------------------------------------------------
// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
detect_features_filtered
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config =
      vital::algo::detect_features::get_configuration();

  // nested algorithm configurations
  vital::algo::detect_features
    ::get_nested_algo_configuration("detector", config, d_->feature_detector);
  vital::algo::filter_features
    ::get_nested_algo_configuration("filter", config, d_->feature_filter);

  return config;
}

// ----------------------------------------------------------------------------
// Set this algorithm's properties via a config block
void
detect_features_filtered
::set_configuration(vital::config_block_sptr config)
{
  // nested algorithm configurations
  vital::algo::detect_features
    ::set_nested_algo_configuration("detector", config, d_->feature_detector);
  vital::algo::filter_features
    ::set_nested_algo_configuration("filter", config, d_->feature_filter);
}

// ----------------------------------------------------------------------------
// Check that the algorithm's configuration vital::config_block is valid
bool
detect_features_filtered
::check_configuration(vital::config_block_sptr config) const
{
  bool detector_valid = vital::algo::detect_features
    ::check_nested_algo_configuration("detector", config);
  bool filter_valid = vital::algo::filter_features
    ::check_nested_algo_configuration("filter", config);
  return detector_valid && filter_valid;
}

/// Extract a set of image features from the provided image
vital::feature_set_sptr
detect_features_filtered
::detect(vital::image_container_sptr image_data,
  vital::image_container_sptr mask) const
{
  if (!d_->feature_detector)
  {
    LOG_ERROR(logger(), "Nested feature detector not initialized.");
    return nullptr;
  }
  auto features = d_->feature_detector->detect(image_data, mask);

  if (!d_->feature_filter)
  {
    LOG_WARN(logger(), "Nested feature filter not initialized.");
  }
  else
  {
    return d_->feature_filter->filter(features);
  }
  return features;
}

} // end namespace core
} // end namespace arrows
} // end namespace kwiver
