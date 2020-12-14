/*ckwg +29
 * Copyright 2016-2017, 2020 by Kitware, Inc.
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

#include "refine_detections_process.h"

#include <vital/algo/refine_detections.h>

#include <sprokit/processes/kwiver_type_traits.h>
#include <sprokit/pipeline/process_exception.h>

namespace kwiver {

create_algorithm_name_config_trait( refiner );

//----------------------------------------------------------------
// Private implementation class
class refine_detections_process::priv
{
public:
  priv();
  ~priv();

   vital::algo::refine_detections_sptr m_refiner;

}; // end priv class


// ==================================================================
refine_detections_process::
refine_detections_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new refine_detections_process::priv )
{
  make_ports();
  make_config();
}


refine_detections_process::
~refine_detections_process()
{
}


// ------------------------------------------------------------------
void
refine_detections_process::
_configure()
{
  scoped_configure_instrumentation();

  vital::config_block_sptr algo_config = get_config();

  // Check config so it will give run-time diagnostic of config problems
  if ( ! vital::algo::refine_detections::check_nested_algo_configuration_using_trait(
         refiner, algo_config ) )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(), "Configuration check failed." );
  }

  vital::algo::refine_detections::set_nested_algo_configuration_using_trait(
    refiner,
    algo_config,
    d->m_refiner );

  if ( ! d->m_refiner )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(), "Unable to create refiner" );
  }
}


// ------------------------------------------------------------------
void
refine_detections_process::
_step()
{
  vital::image_container_sptr image = grab_from_port_using_trait( image );
  vital::detected_object_set_sptr dets = grab_from_port_using_trait( detected_object_set );

  vital::detected_object_set_sptr results;
  {
    scoped_step_instrumentation();

    // Get detections from refiner on image
    results = d->m_refiner->refine( image, dets );
  }

  push_to_port_using_trait( detected_object_set, results );
}


// ------------------------------------------------------------------
void
refine_detections_process::
make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( image, optional );
  declare_input_port_using_trait( detected_object_set, required );

  // -- output --
  declare_output_port_using_trait( detected_object_set, optional );
}


// ------------------------------------------------------------------
void
refine_detections_process::
make_config()
{
  declare_config_using_trait( refiner );
}


// ================================================================
refine_detections_process::priv
::priv()
{
}


refine_detections_process::priv
::~priv()
{
}

} // end namespace kwiver
