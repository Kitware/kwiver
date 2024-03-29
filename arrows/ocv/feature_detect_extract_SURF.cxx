// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV SURF feature detector and extractor wrapper implementation

#include "feature_detect_extract_SURF.h"

#include <vital/vital_config.h>

#ifdef KWIVER_OCV_HAS_SURF

// Include the correct file and unify different namespace locations of SURF type
// across versions
#if KWIVER_OPENCV_VERSION_MAJOR < 3
// 2.4.x header location
#include <opencv2/nonfree/features2d.hpp>
typedef cv::SURF cv_SURF_t;
#else
// 3.x header location
#include <opencv2/xfeatures2d/nonfree.hpp>
typedef cv::xfeatures2d::SURF cv_SURF_t;
#endif

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace ocv {

namespace
{

/// Common ORB private implementation class
class priv
{
public:
  // Cosntructor
  priv()
    : hessian_threshold( 100 )
    , n_octaves( 4 )
    , n_octave_layers( 3 )
    , extended( false )
    , upright( false )
  {
  }

  // Create new algorithm instance from current parameters
  cv::Ptr<cv_SURF_t> create() const
  {
#if KWIVER_OPENCV_VERSION_MAJOR < 3
    return cv::Ptr<cv_SURF_t>(
      new cv_SURF_t( hessian_threshold, n_octaves, n_octave_layers,
                     extended, upright  )
    );
#else
    return cv_SURF_t::create( hessian_threshold, n_octaves, n_octave_layers,
                              extended, upright );
#endif
  }

#if KWIVER_OPENCV_VERSION_MAJOR < 3
  // Update algorithm with current parameter
  void update( cv::Ptr<cv_SURF_t> a ) const
  {
    a->set( "hessianThreshold", hessian_threshold );
    a->set( "nOctaves", n_octaves );
    a->set( "nOctaveLayers", n_octave_layers );
    a->set( "extended", extended );
    a->set( "upright", upright );
  }
#endif

  // Update config block with current parameter values
  void update_config( config_block_sptr config ) const
  {
    config->set_value( "hessian_threshold", hessian_threshold,
                       "Threshold for hessian keypoint detector used in SURF" );
    config->set_value( "n_octaves", n_octaves,
                       "Number of pyramid octaves the keypoint detector will "
                       "use." );
    config->set_value( "n_octave_layers", n_octave_layers,
                       "Number of octave layers within each octave." );
    config->set_value( "extended", extended,
                       "Extended descriptor flag (true - use extended "
                       "128-element descriptors; false - use 64-element "
                       "descriptors)." );
    config->set_value( "upright", upright,
                       "Up-right or rotated features flag (true - do not "
                       "compute orientation of features; false - "
                       "compute orientation)." );
  }

  // Set current values based on config block
  void set_config( config_block_sptr config )
  {
    hessian_threshold = config->get_value<int>( "hessian_threshold" );
    n_octaves = config->get_value<int>( "n_octaves");
    n_octave_layers = config->get_value<int>( "n_octave_layers");
    extended = config->get_value<bool>( "extended" );
    upright = config->get_value<bool>( "upright" );
  }

  // Parameters
  double hessian_threshold;
  int n_octaves;
  int n_octave_layers;
  bool extended;
  bool upright;
};

} // end namespace anonymous

class detect_features_SURF::priv
  : public ocv::priv
{
};

class extract_descriptors_SURF::priv
  : public ocv::priv
{
};

detect_features_SURF
::detect_features_SURF()
  : p_( new priv )
{
  attach_logger("arrows.ocv.SURF");
  detector = p_->create();
}

detect_features_SURF
::~detect_features_SURF()
{
}

vital::config_block_sptr
detect_features_SURF
::get_configuration() const
{
  config_block_sptr config = detect_features::get_configuration();
  p_->update_config( config );
  return config;
}

void
detect_features_SURF
::set_configuration(vital::config_block_sptr config)
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );
  p_->set_config( c );

#if KWIVER_OPENCV_VERSION_MAJOR < 3
  p_->update( detector );
#else
  // Create a new detector rather than update on version 3.
  // Use of the update function requires a dynamic_cast which fails on Mac
  detector = p_->create();
#endif
}

bool
detect_features_SURF
::check_configuration( VITAL_UNUSED vital::config_block_sptr config) const
{
  return true;
}

extract_descriptors_SURF
::extract_descriptors_SURF()
  : p_( new priv )
{
  attach_logger("arrows.ocv.SURF");
  extractor = p_->create();
}

extract_descriptors_SURF
::~extract_descriptors_SURF()
{
}

vital::config_block_sptr
extract_descriptors_SURF
::get_configuration() const
{
  config_block_sptr config = extract_descriptors::get_configuration();
  p_->update_config( config );
  return config;
}

void
extract_descriptors_SURF
::set_configuration(vital::config_block_sptr config)
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );
  p_->set_config( c );

#if KWIVER_OPENCV_VERSION_MAJOR < 3
  p_->update( extractor );
#else
  // Create a new detector rather than update on version 3.
  // Use of the update function requires a dynamic_cast which fails on Mac
  extractor = p_->create();
#endif
}

bool
extract_descriptors_SURF
::check_configuration( VITAL_UNUSED vital::config_block_sptr config) const
{
  return true;
}

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver

#endif
