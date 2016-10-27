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
 * \brief Implementation of target simulating process.
 */

#include "simulate_target_selection_process.h"

#include <sprokit/processes/kwiver_type_traits.h>

namespace kwiver {

// ----------------------------------------------------------------
/**
 * @class simulate_target_selection_process
 * @brief Simulate a target selection process
 *
 * This process is a small test component used to simulate a target
 * selector where there is no real target selector.
 */

class simulate_target_selection_process::priv
{
public:
  priv()
  { }

  // nothing yet
};

// ------------------------------------------------------------------
simulate_target_selection_process
::simulate_target_selection_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new simulate_target_selection_process::priv )
{
  make_ports();
}


simulate_target_selection_process
::~simulate_target_selection_process()
{
}


// ------------------------------------------------------------------
void
simulate_target_selection_process
::_configure()
{
  // Nothing yet
}


// ------------------------------------------------------------------
void
simulate_target_selection_process
::_step()
{
  auto input = grab_from_port_using_trait( detected_object_set );

  vital::bounding_box_d bbox( 0, 0, 0, 0 );
  auto top_person = input->select( "person", 0.8 );

  if (top_person.size() > 0)
  {
    bbox = top_person[0]->bounding_box();
  }

  push_to_port_using_trait( bounding_box, bbox );
}


// ------------------------------------------------------------------
void
simulate_target_selection_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( detected_object_set, required );

  // -- output --
  declare_output_port_using_trait( bounding_box, optional );
}


// ------------------------------------------------------------------
void
simulate_target_selection_process
::make_config()
{
}

} // end namespace
