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

#include "non_maximum_suppression_process.h"

#include <sprokit/processes/kwiver_type_traits.h>
#include <sprokit/pipeline/process_exception.h>

namespace kwiver {

// ==================================================================
non_maximum_suppression_process::
non_maximum_suppression_process( kwiver::vital::config_block_sptr const& config )
  : process( config )
{
  make_ports();
}


non_maximum_suppression_process::
~non_maximum_suppression_process()
{
}


// ------------------------------------------------------------------
void
non_maximum_suppression_process::
_configure()
{
  scoped_configure_instrumentation();

  vital::config_block_sptr config = get_config();
  
  if(config->has_value("scale"))
  {
    m_scale_factor = config->get_value<float>("scale");
  }
  else
  {
    m_scale_factor = 1;
  }
  if(!config->has_value("max_overlap"))
  {
    throw sprokit::invalid_configuration_exception(
      name(), "non_maximum_suppression needs a max_overlap value." );
  }
  m_max_overlap = config->get_value<float>("max_overlap");

}


// ------------------------------------------------------------------
void
non_maximum_suppression_process::
_step()
{
  vital::detected_object_set_sptr dets = grab_from_port_using_trait( detected_object_set );
  dets = dets->clone()->select(); // sort by confidence threshold

  vital::detected_object_set_sptr results(new vital::detected_object_set());

  // Prune first
  for(auto det = dets->begin(); det != dets->end(); det++)
  {
    bool should_add = true;
    for(auto result = results->begin(); result != results->end(); result++)
    {
      kwiver::vital::bounding_box_d det_bbox = (*det)->bounding_box();
      kwiver::vital::bounding_box_d res_bbox = (*result)->bounding_box();
      kwiver::vital::bounding_box_d overlap = kwiver::vital::intersection(det_bbox, res_bbox);
	  
      // Check how much they overlap. Only keep if the overlapped percent isn't too high
	  if((overlap.area() / std::min(det_bbox.area(), res_bbox.area())) > m_max_overlap)
      {
        should_add = false;
		break;
      }
    }
    if (should_add) // It doesn't overlap too much, add it in
    {
      results->add(*det);
    }
  }

  // We've got our detections, now scale
  results->scale(m_scale_factor);

  push_to_port_using_trait( detected_object_set, results );
}


// ------------------------------------------------------------------
void
non_maximum_suppression_process::
make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( detected_object_set, required );

  // -- output --
  declare_output_port_using_trait( detected_object_set, optional );
}

} // end namespace kwiver
