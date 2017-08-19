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
 * \brief Interface to algorithms for warping images
 */

#ifndef VITAL_ALGO_STABILIZE_VIDEO_H
#define VITAL_ALGO_STABILIZE_VIDEO_H

#include <vital/vital_config.h>
#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/homography_f2f.h>

namespace kwiver {
namespace vital {
namespace algo {

/// \brief Abstract base class for image stabilization algorithms.
class VITAL_ALGO_EXPORT stabilize_video
  : public kwiver::vital::algorithm_def<stabilize_video>
{
public:

  /// Return the name of this algorithm.
  static std::string static_type_name() { return "stabilize_video"; }

  /// Stabilize an input video frame by producing a homography
  /**
   * This method implements video stabilization by producing a homography that 
   * warps points from the current frame back to a key frame's coordinate 
   * system.
   *
   * \param[in] ts Time stamp for the input image.
   * \param[in] image_src the source image data to stabilize
   * \param[out] src_to_ref Source to reference homography
   * \param[out] coordinate_system_updated Set to true if this frame establishes a new reference coordinate system.
   */
  virtual void
  process_image( const timestamp& ts,
                 const image_container_sptr image_src,
                 homography_f2f_sptr& src_to_ref,
                 bool&  coordinate_system_updated) = 0;

protected:
  stabilize_video();

};

/// type definition for shared pointer to a stabilize_video algorithm
typedef std::shared_ptr<stabilize_video> stabilize_video_sptr;


} } } // end namespace

#endif // VITAL_ALGO_STABILIZE_VIDEO_H
