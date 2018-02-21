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

#ifndef VITAL_ALGO_WARP_IMAGE_H_
#define VITAL_ALGO_WARP_IMAGE_H_

#include <vital/vital_config.h>
#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/homography.h>

namespace kwiver {
namespace vital {
namespace algo {

/// \brief Abstract base class for image warping algorithms.
class VITAL_ALGO_EXPORT warp_image
  : public kwiver::vital::algorithm_def<warp_image>
{
public:

  /// Return the name of this algorithm.
  static std::string static_type_name() { return "warp_image"; }

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
  warp( image_container_sptr image_src,
        image_container_sptr& image_dest,
        homography_sptr homog) const = 0;

protected:
  warp_image();

};

/// type definition for shared pointer to a warp_image algorithm
typedef std::shared_ptr<warp_image> warp_image_sptr;


} } } // end namespace

#endif /* VITAL_ALGO_WARP_IMAGE_H_ */
