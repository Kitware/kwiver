// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Defaults plugin algorithm registration interface impl

#include <arrows/uuid/kwiver_algo_uuid_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <arrows/uuid/algo/uuid_factory_uuid.h>

namespace kwiver {

namespace arrows {

namespace uuid {

extern "C"
KWIVER_ALGO_UUID_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpm.add_factory< vital::algo::uuid_factory,
    uuid_factory_uuid >( "uuid" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.uuid" );
}

} // namespace uuid

} // namespace arrows

}     // end namespace
