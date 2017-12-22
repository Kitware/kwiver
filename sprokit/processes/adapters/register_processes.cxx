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

#include <sprokit/processes/adapters/kwiver_processes_adapter_export.h>
#include <sprokit/pipeline/process_factory.h>

#include <vital/plugin_loader/plugin_loader.h>

#include "input_adapter_process.h"
#include "output_adapter_process.h"


// ----------------------------------------------------------------
/*! \brief Regsiter processes
 *
 *
 */
extern "C"
KWIVER_PROCESSES_ADAPTER_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )

{
  static auto const module_name = kwiver::vital::plugin_manager::module_t( "kwiver_processes_adapters" );

  if ( sprokit::is_process_module_loaded( vpm, module_name ) )
  {
    return;
  }

  // ----------------------------------------------------------------
  auto fact = vpm.ADD_PROCESS( kwiver::input_adapter_process );
  fact->add_attribute(  kwiver::vital::plugin_factory::PLUGIN_NAME, "input_adapter" )
    .add_attribute(  kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME, module_name )
    .add_attribute(  kwiver::vital::plugin_factory::PLUGIN_DESCRIPTION,
                     "Source process for pipeline.\n\n"
                     "Pushes data items into pipeline ports. "
                     "Ports are dynamically created as needed based on connections specified in the pipeline file." )
    .add_attribute( "no-test", "introspect" ); // do not include in introspection test

  fact = vpm.ADD_PROCESS( kwiver::output_adapter_process );
  fact->add_attribute(  kwiver::vital::plugin_factory::PLUGIN_NAME,  "output_adapter" )
    .add_attribute(  kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME, module_name )
    .add_attribute(  kwiver::vital::plugin_factory::PLUGIN_DESCRIPTION,
                     "Sink process for pipeline.\n\n"
                     "Accepts data items from pipeline ports. "
                     "Ports are dynamically created as needed based on connections specified in the pipeline file." )
    .add_attribute( "no-test", "introspect" ); // do not include in introspection test

  // - - - - - - - - - - - - - - - - - - - - - - -
  sprokit::mark_process_module_as_loaded( vpm, module_name );
}
