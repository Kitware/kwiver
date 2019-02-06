/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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
 * \brief OCV compute_optical_flow_brox algorithm implementation
 */

#ifndef KWIVER_ARROWS_OCV_COMPUTE_OPTICAL_FLOW_BROX_H_
#define KWIVER_ARROWS_OCV_COMPUTE_OPTICAL_FLOW_BROX_H_

#ifdef KWIVER_HAS_OPENCV_VER_3

#include <vital/algo/compute_optical_flow.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>
#include <opencv2/cudaoptflow.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {


class KWIVER_ALGO_OCV_EXPORT compute_optical_flow_brox
    : public vital::algorithm_impl< compute_optical_flow_brox, 
                                    vital::algo::compute_optical_flow >
{
public:
  compute_optical_flow_brox();
  
  virtual ~compute_optical_flow_brox();

  virtual vital::config_block_sptr get_configuration() const;

  virtual void set_configuration( vital::config_block_sptr config );
  virtual bool check_configuration( vital::config_block_sptr config ) const { return true;}

  /// Compute an optical flow image based on a pair of images using Brox algorithm
  /**
  * \param image contains an input image
  * \param successive_image contains a successive image
  * \returns returns the image representation of the optical_flow features
  */
  virtual vital::image_container_sptr 
  compute( vital::image_container_sptr image,
           vital::image_container_sptr successive_image ) const;

private:
  class priv;
  const std::unique_ptr<priv> o;
};
}
}
}

#endif

#endif
