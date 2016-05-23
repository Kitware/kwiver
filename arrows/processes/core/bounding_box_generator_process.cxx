/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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

#include "bounding_box_generator_process.h"

#include <vital/vital_types.h>
#include <arrows/processes/kwiver_type_traits.h>
#include <vital/types/vector.h>
#include <vital/types/detected_object.h>

namespace kwiver {

create_config_trait( upper_left, vital::vector_2d, "0 0", "The upper left point (x y)" );
create_config_trait( lower_right, vital::vector_2d, "2500000 250000", "The lower right point (x y)" );

class bounding_box_generator_process::priv
{
public:
  priv()
  { }

  ~priv()
  { }
  vital::vector_2d m_upper_left;
  vital::vector_2d m_lower_right;
}; //end priv

bounding_box_generator_process
::bounding_box_generator_process( vital::config_block_sptr const& config )
 : process( config ),
   d( new bounding_box_generator_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach
  make_ports();
  make_config();
}


bounding_box_generator_process
::~bounding_box_generator_process()
{
}


void
bounding_box_generator_process::_configure()
{
  d->m_upper_left = config_value_using_trait( upper_left );
  d->m_lower_right = config_value_using_trait( lower_right );
}


void
bounding_box_generator_process::_step()
{
  vital::detected_object::bounding_box result(d->m_upper_left, d->m_lower_right);
  push_to_port_using_trait( bounding_box, result );
}


void
bounding_box_generator_process::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  //output
  declare_output_port_using_trait( bounding_box, optional );
}


void
bounding_box_generator_process::make_config()
{
  declare_config_using_trait( upper_left );
  declare_config_using_trait( lower_right );
}

}//namespace kwiver
