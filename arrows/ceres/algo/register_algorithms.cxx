// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Ceres algorithm registration implementation

#include <arrows/ceres/kwiver_algo_ceres_plugin_export.h>
#include <vital/plugin_management/plugin_factory.h>

#include <arrows/ceres/algo/bundle_adjust.h>
#include <arrows/ceres/algo/optimize_cameras.h>
#include <arrows/ceres/types.h>

namespace kwiver {

namespace arrows {

namespace ceres {

extern "C"
KWIVER_ALGO_CERES_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto const module_name = std::string( "arrows.ceres" );

  auto fact = vpl.add_factory< vital::algo::bundle_adjust,
    kwiver::arrows::ceres::bundle_adjust >( "ceres" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, module_name );

  fact = vpl.add_factory< vital::algo::optimize_cameras,
    kwiver::arrows::ceres::optimize_cameras >( "ceres" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, module_name );
}

} // end namespace ceres

} // end namespace arrows

} // end namespace kwiver
