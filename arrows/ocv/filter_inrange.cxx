/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Implementation of ocv::filter_blur
 */

#include "filter_inrange.h"

#include <vital/exceptions.h>
#include <vital/util/wall_timer.h>

#include <kwiversys/RegularExpression.hxx>
#include <arrows/ocv/image_container.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


/// Private implementation class
class filter_inrange::priv
{
public:
  /// Constructor
  kwiver::vital::rgb_color m_lower_bound;
  kwiver::vital::rgb_color m_upper_bound;
  kwiver::vital::logger_handle_t m_logger;
  kwiver::vital::wall_timer m_timer;

  priv()
    : m_lower_bound(0,0,0),
      m_upper_bound(255,255,255)
  {
  }

  bool parse_color_string(std::string const color_string,
      kwiver::vital::rgb_color &color)
  {
    kwiversys::RegularExpression exp( "([0-9]+) ([0-9]+) ([0-9]+)" );

    if ( ! exp.find( color_string ) )
    {
      return false;
    }

    color.r = std::stoi( exp.match(1) );
    color.g = std::stoi( exp.match(2) );
    color.b = std::stoi( exp.match(3) );

    return true;
  }

  // Apply inrange operation to image
  void
  filter(cv::Mat const &cv_src, cv::Mat &cv_dest)
  {
    LOG_DEBUG( m_logger, "filter_inrange lower bound: " << m_lower_bound );
    LOG_DEBUG( m_logger, "filter_inrange upper bound: " << m_upper_bound );
    cv::Scalar lowerBound( m_lower_bound.r,m_lower_bound.g,m_lower_bound.b );
    cv::Scalar upperBound( m_upper_bound.r,m_upper_bound.g,m_upper_bound.b );
    cv::inRange( cv_src, lowerBound, upperBound, cv_dest );
    LOG_DEBUG( m_logger, "filter_inrange completed");
  }
};


/// Constructor
filter_inrange
::filter_inrange()
: d_(new priv)
{
  attach_logger( "arrows.ocv.filter_inrange" );
  d_->m_logger = logger();
}

/// Destructor
filter_inrange
::~filter_inrange() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
filter_inrange
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  std::ostringstream os_lower;
  os_lower << d_->m_lower_bound;
  config->set_value("lower_bound", os_lower.str(),
                    "Lower bound of range as  3-tuple 0-255" );
  std::ostringstream os_upper;
  os_upper << d_->m_upper_bound;
  config->set_value("upper_bound", os_upper.str(),
                    "Upper bound of range as 3-tuple 0-255" );
  return config;
}


/// Set this algo's properties via a config block
void
filter_inrange
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  std::string lower_string = config->get_value<std::string>("lower_bound");
  d_->parse_color_string(lower_string,d_->m_lower_bound);

  std::string upper_string = config->get_value<std::string>("upper_bound");
  d_->parse_color_string(upper_string,d_->m_upper_bound);

  LOG_DEBUG( logger(), "Lower Bound: " << d_->m_lower_bound);
  LOG_DEBUG( logger(), "Upper Bound: " << d_->m_upper_bound);
}


bool
filter_inrange
::check_configuration(vital::config_block_sptr config) const
{
  kwiver::vital::rgb_color tmp_color;

  std::string lower_string = config->get_value<std::string>("lower_bound");
  if ( ! d_->parse_color_string(lower_string,tmp_color)) {
      LOG_ERROR(d_->m_logger,"Cannot parse lower bound: " << lower_string);
      return false;
  }

  std::string upper_string = config->get_value<std::string>("upper_bound");
  if ( ! d_->parse_color_string(upper_string,tmp_color) ) {
      LOG_ERROR(d_->m_logger,"Cannot parse upper bound: " << upper_string);
      return false;
  }

  return true;
}


/// Warp an input image with a homography
kwiver::vital::image_container_sptr
filter_inrange::
filter( kwiver::vital::image_container_sptr image_data )
{
  LOG_TRACE( logger(), "Starting algorithm" );
  d_->m_timer.start();

  cv::Mat cv_src = ocv::image_container::vital_to_ocv( image_data->get_image() );

  if( cv_src.channels() == 1 )
  {
    // TODO: Figure out why this is necessary. Something is wrong with
    // vital_to_ocv for grayscale images.
    // See issue: https://github.com/Kitware/kwiver/issues/269
    cv_src = cv_src.clone();
  }

  cv::Mat cv_dest;
  d_->filter( cv_src, cv_dest );

  cv_dest = cv_dest.clone();
  LOG_DEBUG(logger(), "Nonzero pixels in dest: " << cv::countNonZero(cv_dest));

  kwiver::vital::image_container_sptr image_dest;
  image_dest = std::make_shared<ocv::image_container>(cv_dest);

  d_->m_timer.stop();
  LOG_TRACE( logger(), "Total processing time: " << d_->m_timer.elapsed() << " seconds");
  return image_dest;
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
