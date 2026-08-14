// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV BRISK feature detector and extractor wrapper implementation

#include "feature_detect_extract_BRISK.h"

#include <vital/vital_config.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

cv::Ptr< cv::BRISK >
detect_features_BRISK
::create() const
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  return cv::Ptr< cv::BRISK >(
    new cv::BRISK(
      this->get_threshold(), this->get_octaves(),
      this->get_pattern_scale() )
  );
#else
  return cv::BRISK::create(
    this->get_threshold(), this->get_octaves(),
    this->get_pattern_scale() );
#endif
}

void
detect_features_BRISK
::initialize()
{
  attach_logger( "arrows.ocv.BRISK" );
  detector = this->create();
}

detect_features_BRISK
::~detect_features_BRISK()
{}

void
detect_features_BRISK
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_detector_parameters();
}

bool
detect_features_BRISK
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

void
detect_features_BRISK
::update_detector_parameters() const
{
  this->detector.constCast< cv::FeatureDetector >() = this->create();
}

// ----------------------------------------------------------------------------
cv::Ptr< cv::BRISK >
extract_descriptors_BRISK
::create() const
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  return cv::Ptr< cv::BRISK >(
    new cv::BRISK(
      this->get_threshold(), this->get_octaves(),
      this->get_pattern_scale() )
  );
#else
  return cv::BRISK::create(
    this->get_threshold(), this->get_octaves(),
    this->get_pattern_scale() );
#endif
}

void
extract_descriptors_BRISK
::initialize()
{
  attach_logger( "arrows.ocv.BRISK" );
  this->extractor = this->create();
}

extract_descriptors_BRISK
::~extract_descriptors_BRISK()
{}

void
extract_descriptors_BRISK
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_extractor_parameters();
}

bool
extract_descriptors_BRISK
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

void
extract_descriptors_BRISK
::update_extractor_parameters() const
{
  this->extractor.constCast< cv::DescriptorExtractor >() = this->create();
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
