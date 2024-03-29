// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV MSER feature detector wrapper implementation

#include "detect_features_MSER.h"

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace ocv {

class detect_features_MSER::priv
{
public:
  /// Constructor
  priv()
    : delta( 5 ),
      min_area( 60 ),
      max_area( 14400 ),
      max_variation( 0.25 ),
      min_diversity( 0.2 ),
      max_evolution( 200 ),
      area_threshold( 1.01 ),
      min_margin( 0.003 ),
      edge_blur_size( 5 )
  {
    #if KWIVER_OPENCV_VERSION_MAJOR >= 3
    pass2only = false;
    #endif
  }

  cv::Ptr<cv::MSER> create() const {
#if KWIVER_OPENCV_VERSION_MAJOR < 3
    // 2.4.x version constructor
    return cv::Ptr<cv::MSER>(
        new cv::MSER( delta, min_area, max_area, max_variation,
                      min_diversity, max_evolution, area_threshold,
                      min_margin, edge_blur_size )
    );
#else
    cv::Ptr<cv::MSER> p =
        cv::MSER::create( delta, min_area, max_area, max_variation,
                          min_diversity, max_evolution, area_threshold,
                          min_margin, edge_blur_size );
    p->setPass2Only( pass2only );
    return p;
#endif
  }

  // OCV 3.x does not have adequate setter functions for updating all parameters
  // the algorithm was constructed with. So, instead of updating, we'll just
  // create a new cv::MSER instance on parameter update.

  /// Update given config block with currently set parameter values
  void update_config( config_block_sptr config ) const
  {
    config->set_value( "delta", delta,
                       "Compares (size[i] - size[i-delta]) / size[i-delta]" );
    config->set_value( "min_area", min_area,
                       "Prune areas smaller than this" );
    config->set_value( "max_area", max_area,
                       "Prune areas larger than this" );
    config->set_value( "max_variation", max_variation,
                       "Prune areas that have similar size to its children" );
    config->set_value( "min_diversity", min_diversity,
                       "For color images, trace back to cut off MSER with "
                         "diversity less than min_diversity" );
    config->set_value( "max_evolution", max_evolution,
                       "The color images, the evolution steps." );
    config->set_value( "area_threshold", area_threshold,
                       "For color images, the area threshold to cause "
                         "re-initialization" );
    config->set_value( "min_margin", min_margin,
                       "For color images, ignore too-small regions." );
    config->set_value( "edge_blur_size", edge_blur_size,
                       "For color images, the aperture size for edge blur" );
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
    config->set_value( "pass2only", pass2only, "Undocumented" );
#endif
  }

  /// Set parameter values based on given config block
  void set_config( config_block_sptr const &c )
  {
    delta = c->get_value<int>("delta");
    min_area = c->get_value<int>("min_area");
    max_area = c->get_value<int>("max_area");
    max_variation = c->get_value<double>("max_variation");
    min_diversity = c->get_value<double>("min_diversity");
    max_evolution = c->get_value<int>("max_evolution");
    area_threshold = c->get_value<double>("area_threshold");
    min_margin = c->get_value<double>("min_margin");
    edge_blur_size = c->get_value<int>("edge_blur_size");
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
    pass2only = c->get_value<bool>("pass2only");
#endif
  }

  /// Check config parameter values
  bool check_config(vital::config_block_sptr const &c,
                    logger_handle_t const &logger) const
  {
    bool valid = true;

    // checking that area values are >= 0
    if( c->get_value<int>("min_area") < 0 ||
        c->get_value<int>("max_area") < 0 ||
        c->get_value<double>("area_threshold") < 0 )
    {
      LOG_ERROR(logger, "Areas should be at least 0.");
      valid = false;
    }

    return valid;
  }

  /// Parameters
  int delta;
  int min_area;
  int max_area;
  double max_variation;
  double min_diversity;
  int max_evolution;
  double area_threshold;
  double min_margin;
  int edge_blur_size;
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
  bool pass2only;
#endif
};

detect_features_MSER
::detect_features_MSER()
  : p_( new priv )
{
  attach_logger("arrows.ocv.detect_features_FAST");
  detector = p_->create();
}

detect_features_MSER
::~detect_features_MSER()
{
}

vital::config_block_sptr
detect_features_MSER
::get_configuration() const
{
  config_block_sptr config = ocv::detect_features::get_configuration();
  p_->update_config( config );
  return config;
}

void detect_features_MSER
::set_configuration(vital::config_block_sptr config)
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );
  p_->set_config( c );
  detector = p_->create();
}

bool
detect_features_MSER
::check_configuration(vital::config_block_sptr config) const
{
  config_block_sptr c = get_configuration();
  c->merge_config(config);
  return p_->check_config( c, logger() );
}

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
