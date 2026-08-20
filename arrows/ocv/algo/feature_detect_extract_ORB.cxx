// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV ORB feature detector and extractor wrapper implementation

#include "feature_detect_extract_ORB.h"

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

bool
check_configuration_common(
  vital::config_block_sptr config,
  kwiver::vital::logger_handle_t logger )
{
  bool valid = true;
  // Must be one of the enumeration values
  int temp = config->get_value< int >( "score_type" );
  if( !( temp == cv::ORB::HARRIS_SCORE ||
         temp == cv::ORB::FAST_SCORE ) )
  {
    LOG_ERROR(
      logger, "Score type not a valid enumeration value. Must be "
              "either " << cv::ORB::HARRIS_SCORE <<
        " for cv::ORB::HARRIS_SCORE or "
                        << cv::ORB::FAST_SCORE <<
        " for cv::ORB::FAST_SCORE." );
    valid = false;
  }

  return valid;
}

cv::Ptr< cv::ORB >
create_common(
  int n_features, float scale_factor, int n_levels,
  int edge_threshold, int first_level, int wta_k, int score_type,
  int patch_size, int fast_threshold = 0 )
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  return cv::Ptr< cv::ORB >(
    new cv::ORB(
      n_features, scale_factor, n_levels, edge_threshold,
      first_level, wta_k, score_type, patch_size ) );
#else
  return cv::ORB::create(
    n_features, scale_factor, n_levels, edge_threshold,
    first_level, wta_k, orb::ScoreType( score_type ), patch_size,
    fast_threshold );
#endif
}

/// Update algo with current parameter values
void
update_common(
  cv::Ptr< cv::ORB > orb,
  int n_features, float scale_factor, int n_levels, int edge_threshold,
  int first_level, int wta_k, int score_type, int patch_size,
  int fast_threshold = 0 )
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  orb->set( "nFeatures", n_features );
  orb->set( "scaleFactor", scale_factor );
  orb->set( "nLevels", n_levels );
  orb->set( "firstLevel", first_level );
  orb->set( "edgeThreshold", edge_threshold );
  orb->set( "patchSize", patch_size );
  orb->set( "WTA_K", wta_k );
  orb->set( "scoreType", score_type );
#else
  orb->setMaxFeatures( n_features );
  orb->setScaleFactor( scale_factor );
  orb->setNLevels( n_levels );
  orb->setEdgeThreshold( edge_threshold );
  orb->setFirstLevel( first_level );
  orb->setWTA_K( wta_k );
  orb->setScoreType( orb::ScoreType( score_type ) );
  orb->setPatchSize( patch_size );
  orb->setFastThreshold( fast_threshold );
#endif
}

} // namespace

// -------------------------------------------------------------------------------------
void
detect_features_ORB
::initialize()
{
  attach_logger( "arrows.ocv.ORB" );
  detector = create_common(
    get_n_features(), get_scale_factor(), get_n_levels(), get_edge_threshold(),
    get_first_level(), get_wta_k(), get_score_type(), get_patch_size()
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
    , get_fast_threshold()
#endif
  );
}

detect_features_ORB
::~detect_features_ORB()
{}

void
detect_features_ORB
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_detector_parameters();
}

void
detect_features_ORB
::update_detector_parameters() const
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  update_common(
    detector,
    get_n_features(), get_scale_factor(), get_n_levels(), get_edge_threshold(),
    get_first_level(), get_wta_k(), get_score_type(), get_patch_size() );
#else
  cv::Ptr< cv::ORB > orb  = detector.dynamicCast< cv::ORB >();
  update_common(
    orb,
    get_n_features(), get_scale_factor(), get_n_levels(), get_edge_threshold(),
    get_first_level(), get_wta_k(), get_score_type(), get_patch_size(),
    get_fast_threshold() );
#endif
}

bool
detect_features_ORB
::check_configuration( vital::config_block_sptr config ) const
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );
  return check_configuration_common( c, logger() );
}

// -----------------------------------------------------------------------
void
extract_descriptors_ORB
::initialize()
{
  attach_logger( "arrows.ocv.ORB" );
  extractor = create_common(
    get_n_features(), get_scale_factor(), get_n_levels(), get_edge_threshold(),
    get_first_level(), get_wta_k(), get_score_type(), get_patch_size()
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
    , get_fast_threshold()
#endif
  );
}

extract_descriptors_ORB
::~extract_descriptors_ORB()
{}

void
extract_descriptors_ORB
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_extractor_parameters();
}

void
extract_descriptors_ORB
::update_extractor_parameters() const
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  update_common(
    extractor,
    get_n_features(), get_scale_factor(), get_n_levels(), get_edge_threshold(),
    get_first_level(), get_wta_k(), get_score_type(), get_patch_size() );
#else
  cv::Ptr< cv::ORB > orb  = extractor.dynamicCast< cv::ORB >();
  update_common(
    orb,
    get_n_features(), get_scale_factor(), get_n_levels(), get_edge_threshold(),
    get_first_level(), get_wta_k(), get_score_type(), get_patch_size(),
    get_fast_threshold() );
#endif
}

bool
extract_descriptors_ORB
::check_configuration( vital::config_block_sptr config ) const
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );
  return check_configuration_common( c, logger() );
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
