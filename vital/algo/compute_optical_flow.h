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
 * \brief compute_optical_flow algorithm definition
 */

#ifndef VITAL_ALGO_COMPUTE_OPTICAL_FLOW_H_
#define VITAL_ALGO_COMPUTE_OPTICAL_FLOW_H_

#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>

namespace kwiver {
namespace vital {
namespace algo {

/// An abstract class for computing an image based representation of optical flow 
/// features associated with a pair of images
class VITAL_ALGO_EXPORT compute_optical_flow
    : public kwiver::vital::algorithm_def<compute_optical_flow>
{
public:
    /// Return the name of the algorithm
    static std::string static_type_name() { return "compute_optical_flow"; }

    /// Compute an optical flow image give a pair of images
    /**
     * \param image contains an input image 
     * \param successive_image contains a successive image 
     * \return returns the image representation of the optical flow features
     */
    virtual kwiver::vital::image_container_sptr
    compute( const kwiver::vital::image_container_sptr image,
             const kwiver::vital::image_container_sptr successive_image) const = 0;
protected:
    compute_optical_flow();

};

typedef std::shared_ptr<compute_optical_flow> compute_optical_flow_sptr;

} 
}
} // end namespace

#endif // VITAL_ALGO_COMPUTE_OPTICAL_FLOW_H_
