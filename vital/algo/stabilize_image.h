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

#ifndef VITAL_ALGO_STABILIZE_IMAGE_H
#define VITAL_ALGO_STABILIZE_IMAGE_H

#include <vital/vital_config.h>
#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/homography_f2f.h>

namespace kwiver {
namespace vital {
namespace algo {

/// \brief Abstract base class for image stabilization algorithms.
class VITAL_ALGO_EXPORT stabilize_image
  : public kwiver::vital::algorithm_def<stabilize_image>
{
public:

  /// Return the name of this algorithm.
  static std::string static_type_name() { return "stabilize_image"; }

  /// Stabilize an input image by producing a homography
  /**
   * This method implements image stabilization by producing a
   * homography to relate the input image to a reference image.
   *
   * \param ts Time stamp for the input image.
   * \param image_src the source image data to stabilize
   *
   * \return
   */
  virtual homography_f2f_sptr
    stabilize( const timestamp& ts,
               const image_container_sptr image_src ) = 0;

protected:
  stabilize_image();

};

/// type definition for shared pointer to a stabilize_image algorithm
typedef std::shared_ptr<stabilize_image> stabilize_image_sptr;


} } } // end namespace

#endif // VITAL_ALGO_STABILIZE_IMAGE_H
