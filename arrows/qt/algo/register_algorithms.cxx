// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Register Qt algorithms implementation

#include <arrows/qt/kwiver_algo_qt_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <arrows/qt/algo/image_io.h>

namespace kwiver {

namespace arrows {

namespace qt {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_QT_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  static auto const module_name = std::string( "arrows.qt" );

  if( vpm.is_module_loaded( module_name ) )
  {
    return;
  }

  auto fact =
    vpm.add_factory< vital::algo::image_io, image_io >( "qt" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_qt" );

  vpm.mark_module_as_loaded( module_name );
}

} // end namespace qt

} // end namespace arrows

} // end namespace kwiver
