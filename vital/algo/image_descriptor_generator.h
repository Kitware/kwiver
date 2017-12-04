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
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Header defining abstract image descriptor generator algorithm
 */

#ifndef VITAL_ALGO_IMAGE_DESCRIPTOR_GENERATOR_H_
#define VITAL_ALGO_IMAGE_DESCRIPTOR_GENERATOR_H_

#include <vital/algo/algorithm.h>
#include <vital/types/descriptor.h>
#include <vital/types/descriptor_set.h>
#include <vital/types/image_container.h>


namespace kwiver {
namespace vital {
namespace algo {

/**
 * @brief Image descriptor generator algorithm interface.
 *
 * Image descriptor generator algorithms take in an image and describe as a
 * vector of floating-point values.  Descriptor generator algorithms, once
 * configured, always output descriptors of the same dimensionality.
 */
class VITAL_ALGO_EXPORT image_descriptor_generator
  : public algorithm_def<image_descriptor_generator>
{
public:
  /// Return the name of this algorithm interface.
  static std::string static_type_name() { return "image_descriptor_generator"; }

  /**
   * Describe the single input image.
   *
   * \param image Input image to describe.
   * \returns Generated descriptor.
   */
  virtual descriptor_sptr
    compute_descriptor( image_container_sptr image ) const = 0;

  /**
   * Describe multiple input images.
   *
   * \param images Set of input images to describe.
   * \returns Set of descriptors in parallel order to input images.
   */
  virtual descriptor_set_sptr
    compute_descriptors( image_container_sptr_list images ) const = 0;

protected:
  image_descriptor_generator();
};


/// Shared pointer to generic image descriptor type.
typedef std::shared_ptr< image_descriptor_generator > image_descriptor_generator_sptr;


} // end namespace algo
} // end namespace vital
} // end namespace kwiver


#endif //VITAL_ALGO_IMAGE_DESCRIPTOR_GENERATOR_H_
