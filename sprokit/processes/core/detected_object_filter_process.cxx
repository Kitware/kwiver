/*ckwg +29
 * Copyright 2017, 2020 by Kitware, Inc.
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
 * \brief Implementation of the detected object set filter process.
 */

#include "detected_object_filter_process.h"

#include <vital/algo/detected_object_filter.h>
#include <sprokit/processes/kwiver_type_traits.h>
#include <sprokit/pipeline/process_exception.h>

#include <sstream>
#include <iostream>

namespace kwiver {

create_algorithm_name_config_trait( filter );

// ----------------------------------------------------------------
/**
 * \class detected_object_filter_process
 *
 * \brief Filter detected image object sets.
 *
 * \process This process filters a set of detected image objects and
 * produces a new set of detected image objects. The actual processing
 * is done by the selected \b detected_object_filter algorithm
 * implementation.
 *
 * \iports
 *
 * \iport{detected_object_set} Set of objects to be passed to the
 * filtering algorithm.
 *
 * \oports
 *
 * \oport{detected_object_set} SEt of objects produced by the
 * filtering algorithm.
 *
 * \configs
 *
 * \config{filter} Name of the configuration subblock that selects
 * and configures the drawing algorithm.
 */

//----------------------------------------------------------------
// Private implementation class
class detected_object_filter_process::priv
{
public:
  priv();
  ~priv();

  vital::algo::detected_object_filter_sptr m_filter;

}; // end priv class


// ================================================================

detected_object_filter_process
::detected_object_filter_process( kwiver::vital::config_block_sptr const& config )
  : process( config )
  , d( new detected_object_filter_process::priv )
{
  make_ports();
  make_config();
}


detected_object_filter_process
::~detected_object_filter_process()
{
}


// ----------------------------------------------------------------
void
detected_object_filter_process
::_configure()
{
  scoped_configure_instrumentation();

  vital::config_block_sptr algo_config = get_config();

  // Check config so it will give run-time diagnostic of config problems
  if ( ! vital::algo::detected_object_filter::check_nested_algo_configuration_using_trait(
         filter, algo_config ) )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(), "Configuration check failed." );
  }

  vital::algo::detected_object_filter::set_nested_algo_configuration_using_trait(
    filter,
    algo_config,
    d->m_filter );

  if ( ! d->m_filter )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(), "Unable to create filter" );
  }
}


// ----------------------------------------------------------------
void
detected_object_filter_process
::_step()
{
  vital::detected_object_set_sptr input = grab_from_port_using_trait( detected_object_set );

  vital::detected_object_set_sptr result;

  {
    scoped_step_instrumentation();

    result = d->m_filter->filter( input );
  }

  push_to_port_using_trait( detected_object_set, result );
}


// ----------------------------------------------------------------
void
detected_object_filter_process
::make_ports()
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


// ----------------------------------------------------------------
void
detected_object_filter_process
::make_config()
{
  declare_config_using_trait( filter );
}


// ================================================================
detected_object_filter_process::priv
::priv()
{
}


detected_object_filter_process::priv
::~priv()
{
}

} //end namespace
