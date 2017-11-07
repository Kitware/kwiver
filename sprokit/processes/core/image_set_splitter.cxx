/*ckwg +29
 * Copyright 2016-2017 by Kitware, Inc.
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


#include "image_set_splitter.h"

#include <kwiver_type_traits.h>

#include <vital/algo/image_io.h>
#include <vital/types/image_container.h>
#include <vital/types/image_container_set.h>
#include <vital/vital_types.h>
#include <trait_utils.h>

namespace kwiver {

class image_set_splitter::priv
{
public:
  priv() {}
  ~priv() {}
}; // end priv class

// =============================================================================

image_set_splitter
::image_set_splitter( kwiver::vital::config_block_sptr const& config )
        : process( config ),
          d( new image_set_splitter::priv )
{
  // Attach our logger name to process logger
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}

image_set_splitter
::~image_set_splitter()
{
}


// ----------------------------------------------------------------
void
image_set_splitter
::_configure()
{
}


// ----------------------------------------------------------------
void
image_set_splitter
::_step()
{
  vital::image_container_set_sptr input = grab_from_port_using_trait( image_set );

  for (auto img : input->images())
    push_to_port_using_trait( image, img );
}


// -----------------------------------------------------------------------------
void image_set_splitter
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  declare_input_port_using_trait( image_set, required );
//  declare_input_port_using_trait( timestamp, optional,
//    "Image timestamp, optional. The frame number from this timestamp is used "
//    "to number the output files. If the timestamp is not connected or not "
//    "valid, the output files are sequentially numbered from 1." );

  declare_output_port_using_trait( image, required );
}


// -----------------------------------------------------------------------------
void image_set_splitter
::make_config()
{
}


} // end namespace