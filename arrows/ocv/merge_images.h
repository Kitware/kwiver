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
 * \brief Header for OCV merge_images algorithm
 */

#pragma once

#include <arrows/ocv/kwiver_arrows_ocv_export.h>

#include <vital/algo/merge_images.h>

namespace kwiver {
namespace arrows {
namespace ocv {

/// A class for writing out image chips around detections, useful as a debugging process
/// for ensuring that the refine detections process is running on desired ROIs.
class KWIVER_ARROWS_OCV_EXPORT merge_images
  : public vital::algorithm_impl<merge_images, vital::algo::merge_images>
{
public:
  PLUGIN_INFO( "ocv",
               "Merge two images into one using opencv functions" )

  /// Constructor
  merge_images();

  /// Destructor
  virtual ~merge_images();

  virtual void set_configuration( kwiver::vital::config_block_sptr ) { }
  virtual bool check_configuration( kwiver::vital::config_block_sptr config )
    const { return true; }

  /// Merge images
  virtual kwiver::vital::image_container_sptr
  merge(kwiver::vital::image_container_sptr image1,
        kwiver::vital::image_container_sptr image2) const;
};

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
