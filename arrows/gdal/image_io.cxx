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
 * \brief GDAL image_io implementation
 */

#include "image_io.h"

#include <arrows/gdal/image_container.h>

#include <vital/exceptions/algorithm.h>

namespace kwiver {
namespace arrows {
namespace gdal {

/// Load image image from the file
/**
 * \param filename the path to the file the load
 * \returns an image container refering to the loaded image
 */
vital::image_container_sptr
image_io
::load_(const std::string& filename) const
{
  return vital::image_container_sptr( new gdal::image_container( filename ) );
}


/// Save image image to a file
/**
 * \param filename the path to the file to save.
 * \param data The image container refering to the image to write.
 */
void
image_io
::save_(const std::string& filename,
       vital::image_container_sptr data) const
{
  VITAL_THROW( vital::algorithm_exception, this->type_name(), this->impl_name(),
               "Saving to file not supported." );
}

} // end namespace gdal
} // end namespace arrows
} // end namespace kwiver
