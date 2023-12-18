// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Register KLV algorithm implementations.

#include <arrows/klv/kwiver_algo_klv_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

// interfaces
#include <vital/algo/buffered_metadata_filter.h>
#include <vital/algo/metadata_filter.h>


// implementations
#include <arrows/klv/apply_child_klv.h>
#include <arrows/klv/update_klv.h>


namespace kwiver::arrows::klv {

extern "C"
KWIVER_ALGO_KLV_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact =
    vpl.add_factory< vital::algo::metadata_filter , apply_child_klv >("apply_child_klv");
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_klv" );
  
  fact =
    vpl.add_factory< vital::algo::buffered_metadata_filter , update_klv >("update_klv");
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_klv" );
  
}

} // end namespace
