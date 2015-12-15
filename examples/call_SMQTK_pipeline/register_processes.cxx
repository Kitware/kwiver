/*ckwg +29
 * Copyright 2015 by Kitware, Inc.
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

#include <sprokit/pipeline/process_registry.h>

// -- list processes to register --
#include "supply_image.h"
#include "accept_descriptor.h"
#include "smqtk_extract_export.h"

extern "C"
SMQTK_EXTRACT_EXPORT void register_processes();

// ----------------------------------------------------------------
/** \brief Regsiter processes
 *
 *
 */
void register_processes()
{
  static sprokit::process_registry::module_t const module_name =
    sprokit::process_registry::module_t( "SMQTK_extract" );

  sprokit::process_registry_t const registry( sprokit::process_registry::self() );

  if ( registry->is_module_loaded( module_name ) )
  {
    return;
  }

  // ----------------------------------------------------------------
  registry->register_process( "supply_image", "Supplies a single image",
                              sprokit::create_process< kwiver::supply_image > );

  registry->register_process( "accept_descriptor", "Reads a single vector",
                              sprokit::create_process< kwiver::accept_descriptor > );

  // - - - - - - - - - - - - - - - - - - - - - - -
  registry->mark_module_as_loaded( module_name );
}
