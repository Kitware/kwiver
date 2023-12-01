// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Register KLV applets.

#include <arrows/klv/applets/compare_klv.h>
#include <arrows/klv/applets/kwiver_algo_klv_applets_export.h>

#include <vital/plugin_management/plugin_loader.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_KLV_APPLETS_EXPORT
void
register_factories( vital::plugin_loader& vpm )
{
  
  using namespace kwiver::tools;
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact =
    vpm.add_factory< kwiver_applet, compare_klv >( "compare-klv" );
    fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_klv_applets" )
    .add_attribute( kvpf::ALGORITHM_CATEGORY, kvpf::APPLET_CATEGORY );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
