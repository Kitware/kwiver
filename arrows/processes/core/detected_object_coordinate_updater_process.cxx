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

/**
 * \file
 * \brief Implementation of detected object updater process.
 */

#include "detected_object_coordinate_updater_process.h"

#include <vital/vital_types.h>
#include <arrows/processes/kwiver_type_traits.h>
#include <vital/types/vector.h>
#include <vital/types/detected_object.h>

namespace kwiver {

detected_object_coordinate_updater_process
::detected_object_coordinate_updater_process( vital::config_block_sptr const& config )
  : process( config )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach
  make_ports();
}


detected_object_coordinate_updater_process
::~detected_object_coordinate_updater_process()
{
}


// ------------------------------------------------------------------
void
detected_object_coordinate_updater_process
::_step()
{
  vital::detected_object_set_sptr input = grab_from_port_using_trait( detected_object_set );
  vital::detected_object::bounding_box bbox = grab_from_port_using_trait( bounding_box );

  vital::vector_2d upper_left = bbox.upper_left();

  for ( vital::detected_object_set::iterator iter = input->get_iterator(); ! iter.is_end(); ++iter )
  {
    vital::detected_object_sptr dos = iter.get_object();
    dos->set_bounding_box( dos->get_bounding_box().translate( upper_left ) );
  }

  push_to_port_using_trait( detected_object_set, input );
}


// ------------------------------------------------------------------
void
detected_object_coordinate_updater_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // input
  declare_input_port_using_trait( bounding_box, required );
  declare_input_port_using_trait( detected_object_set, required );

  // output
  declare_output_port_using_trait( detected_object_set, optional );
}

} // namespace kwiver
