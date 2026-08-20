// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV LUCID descriptor extractor wrapper implementation

#include "extract_descriptors_LUCID.h"

#include <vital/vital_config.h>

#ifdef HAVE_OPENCV_XFEATURES2D

#include <opencv2/xfeatures2d.hpp>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

cv::Ptr< cv::xfeatures2d::LUCID >
create( const extract_descriptors_LUCID& parent )
{
  return cv::xfeatures2d::LUCID::create(
    parent.get_lucid_kernel(),
    parent.get_blur_kernel() );
}

} // namespace

void
extract_descriptors_LUCID
::initialize()
{
  attach_logger( "arrows.ocv.LUCID" );
  extractor = create( *this );
}

extract_descriptors_LUCID
::~extract_descriptors_LUCID()
{}

void
extract_descriptors_LUCID
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_extractor_parameters();
}

bool
extract_descriptors_LUCID
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

void
extract_descriptors_LUCID
::update_extractor_parameters() const
{
  extractor.constCast< cv::DescriptorExtractor >() = create( *this );
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // HAVE_OPENCV_XFEATURES2D
