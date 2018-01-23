/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER544
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Implementation of ocv::image_test_mask_proportion
 */

#include "image_test_count_nonzero.h"

#include <vital/exceptions.h>
#include <vital/util/wall_timer.h>
#include <kwiversys/RegularExpression.hxx>
#include <arrows/ocv/image_container.h>

#include <opencv2/imgproc/imgproc.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


/// Private implementation class
class image_test_count_nonzero::priv
{
public:
  /// Constructor
  kwiver::vital::logger_handle_t m_logger;
  kwiver::vital::wall_timer m_timer;
  int m_greater_than_value;
  int m_less_than_value;

  priv()
    : m_greater_than_value(0), m_less_than_value(INT_MAX)
  {
  }

  //
  bool
  test_count_nonzero(cv::Mat const &cv_src)
  {
    int num_pixels = cv::countNonZero(cv_src);

    return num_pixels > m_greater_than_value && num_pixels < m_less_than_value;
  }
};


/// Constructor
image_test_count_nonzero
::image_test_count_nonzero()
: d_(new priv)
{
  attach_logger( "arrows.ocv.image_test_count_nonzero" );
  d_->m_logger = logger();
}

/// Destructor
image_test_count_nonzero
::~image_test_count_nonzero() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
image_test_count_nonzero
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value("greater_than_value", d_->m_greater_than_value,
                    "Non zero pixels must be greater than this value to pass.  Default 0"
                    );
  config->set_value("less_than_value", d_->m_less_than_value,
                    "Non zero pixels must be less than this value to pass.  Default MAX_INT"
                    );
  return config;
}


/// Set this algo's properties via a config block
void
image_test_count_nonzero
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  d_->m_greater_than_value = config->get_value<int>("greater_than_value");
  d_->m_less_than_value = config->get_value<int>("less_than_value");

  LOG_DEBUG( logger(), "m_greater_than_value: " << d_->m_greater_than_value);
  LOG_DEBUG( logger(), "m_less_than_value: " << d_->m_less_than_value);
}


bool
image_test_count_nonzero
::check_configuration(vital::config_block_sptr in_config) const
{
  return true;
}


/// Warp an input image with a homography
bool
image_test_count_nonzero::
test_image( kwiver::vital::image_container_sptr image_data )
{
  LOG_TRACE( logger(), "Starting algorithm" );
  d_->m_timer.start();

  if ( !image_data )
  {
    throw vital::invalid_data("Inputs to ocv::image_test_count_nonzero are null");
  }

  cv::Mat cv_src = ocv::image_container::vital_to_ocv( image_data->get_image() );

  if( cv_src.channels() == 1 )
  {
    // TODO: Figure out why this is necessary. Something is wrong with
    // vital_to_ocv for grayscale images.
    cv_src = cv_src.clone();
  }

	bool test_result = false;
  test_result = d_->test_count_nonzero( cv_src );

  d_->m_timer.stop();
  LOG_TRACE( logger(), "Total processing time: " << d_->m_timer.elapsed() << " seconds");
  return test_result;
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
