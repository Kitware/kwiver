// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <sprokit/processes/adapters/kwiver_processes_adapter_export.h>
#include <sprokit/pipeline/process_factory.h>

#include <vital/plugin_management/plugin_loader.h>

#include "input_adapter_process.h"
#include "output_adapter_process.h"

// ----------------------------------------------------------------
/** \brief Register processes
 *
 *
 */
extern "C"
KWIVER_PROCESSES_ADAPTER_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  using namespace sprokit;

  process_registrar reg( vpm, "kwiver_processes_adapters" );

  if ( reg.is_module_loaded() )
  {
    return;
  }

  reg.register_process< kwiver::input_adapter_process >( process_registrar::no_test );
  reg.register_process< kwiver::output_adapter_process >( process_registrar::no_test );

  // - - - - - - - - - - - - - - - - - - - - - - -
  reg.mark_module_as_loaded();
}
