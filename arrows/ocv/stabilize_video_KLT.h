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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
 * \brief OCV warp_image algorithm impl interface
 */

#ifndef KWIVER_ARROWS_OCV_STABILIZE_VIDEO_KLT_H_
#define KWIVER_ARROWS_OCV_STABILIZE_VIDEO_KLT_H_

#include <memory>

#include <opencv2/opencv.hpp>

#include <vital/types/timestamp.h>
#include <vital/vital_config.h>
#include <vital/algo/stabilize_video.h>
#include <vital/config/config_block.h>

#include <arrows/ocv/kwiver_algo_ocv_export.h>

namespace kwiver {
namespace arrows {
namespace ocv {

/// OCV implementation of warp_image using cv::calcOpticalFlowPyrLK
class KWIVER_ALGO_OCV_EXPORT stabilize_video_KLT
  : public vital::algorithm_impl<stabilize_video_KLT, 
                                 vital::algo::stabilize_video>
{
public:

  /// Constructor
  stabilize_video_KLT();
  /// Destructor
  virtual ~stabilize_video_KLT() VITAL_NOTHROW;

  /// Get this algorithm's \link kwiver::vital::config_block configuration block \endlink
  virtual vital::config_block_sptr get_configuration() const;
  /// Set this algorithm's properties via a config block
  virtual void set_configuration(vital::config_block_sptr config);
  /// Check that the algorithm's configuration vital::config_block is valid
  virtual bool check_configuration(vital::config_block_sptr config) const;

  /// Warp an input image with a homography
  /**
   * This method implements warping an image by a homography.
   * The \p image_src is warped by \p homog and the output pixels are stored in
   * \image_dest.  If an image passed in as \p image_dest the output will be
   * written to that memory, if \p image_dest is nullptr then the algorithm will
   * allocate new image memory for the output.
   *
   * \param[in]     image_src the source image data to warp
   * \param[in,out] image_data the destination image to store the warped output
   * \param[in]     homog homography matrix to apply
   */
  virtual void
  process_image( const kwiver::vital::timestamp& ts,
                 const kwiver::vital::image_container_sptr image_src,
                 kwiver::vital::homography_f2f_sptr& src_to_ref,
                 bool& coordinate_system_updated);

private:
  // private implementation class
  class priv;
  std::unique_ptr<priv> d_;
};

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver

#endif /* KWIVER_ARROWS_OCV_STABILIZE_VIDEO_KLT_H_ */
