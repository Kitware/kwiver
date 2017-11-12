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
 * \brief Implementation for detected_object_set_output process
 */

#include "merge_detected_object_sets_process.h"

#include <vital/vital_types.h>
#include <vital/exceptions.h>
#include <vital/algo/detected_object_set_output.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace algo = kwiver::vital::algo;

namespace kwiver {

create_port_trait( detected_object_set1, detected_object_set,
                   "First detected_object_set" );
create_port_trait( detected_object_set2, detected_object_set,
                   "Second detected_object_set" );

//----------------------------------------------------------------
// Private implementation class
class merge_detected_object_sets_process::priv
{
public:
  priv();
  ~priv();
}; // end priv class


// ================================================================

merge_detected_object_sets_process
::merge_detected_object_sets_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new merge_detected_object_sets_process::priv )
{
  // Attach our logger name to process logger
  attach_logger( kwiver::vital::get_logger( name() ) );

  make_ports();
  make_config();
}


merge_detected_object_sets_process
::~merge_detected_object_sets_process()
{
}


// ----------------------------------------------------------------
void merge_detected_object_sets_process
::_configure()
{
}


// ----------------------------------------------------------------
void merge_detected_object_sets_process
::_step()
{
  vital::detected_object_set_sptr input1 = grab_from_port_using_trait( detected_object_set1 );
  vital::detected_object_set_sptr input2 = grab_from_port_using_trait( detected_object_set2 );
  
  // Get list of all detections from the input sets.
  // TODO: determine whether the clone is necessary.
  auto det_out = input1->clone()->select();
  auto det2 = input2->clone()->select();
  
  det_out.insert( det_out.end(), det2.begin(), det2.end() );

  auto result = std::make_shared<vital::detected_object_set>(det_out);

  push_to_port_using_trait( detected_object_set, result );
}


// ----------------------------------------------------------------
void merge_detected_object_sets_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( detected_object_set1, required );
  declare_input_port_using_trait( detected_object_set2, required );
  
  // -- output --
  declare_output_port_using_trait( detected_object_set, optional );
}


// ----------------------------------------------------------------
void merge_detected_object_sets_process
::make_config()
{
}


// ================================================================
merge_detected_object_sets_process::priv
::priv()
{
}


merge_detected_object_sets_process::priv
::~priv()
{
}

} // end namespace
