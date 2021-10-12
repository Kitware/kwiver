/*ckwg +29
 * Copyright 2014-2016 by Kitware, Inc.
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

#include "extract_descriptors.h"

#include <arrows/viscl/image_container.h>
#include <arrows/viscl/feature_set.h>
#include <arrows/viscl/descriptor_set.h>

#include <viscl/tasks/BRIEF.h>


namespace kwiver {
namespace arrows {
namespace vcl {

/// Private implementation class
class extract_descriptors::priv
{
public:
  /// Constructor
  priv()
  {
  }

  viscl::brief<10> brief;
};


/// Constructor
extract_descriptors
::extract_descriptors()
: d_(new priv)
{
}


/// Destructor
extract_descriptors
::~extract_descriptors()
{
}


/// Extract from the image a descriptor corresoponding to each feature
vital::descriptor_set_sptr
extract_descriptors
::extract(vital::image_container_sptr image_data,
          vital::feature_set_sptr features,
          vital::image_container_sptr /* image_mask */) const
{
  if( !image_data || !features )
  {
    return vital::descriptor_set_sptr();
  }

  viscl::image img = vcl::image_container_to_viscl(*image_data);
  vcl::feature_set::type fs = vcl::features_to_viscl(*features);
  viscl::buffer descriptors;
  d_->brief.compute_descriptors(img, fs.features_, features->size(), descriptors);
  return vital::descriptor_set_sptr(new vcl::descriptor_set(descriptors));
}


} // end namespace vcl
} // end namespace arrows
} // end namespace kwiver
