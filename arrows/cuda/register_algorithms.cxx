// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Register depth algorithms implementation

#include <arrows/cuda/kwiver_algo_cuda_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <arrows/cuda/algo/integrate_depth_maps.h>

namespace kwiver {

namespace arrows {

namespace cuda {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_CUDA_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpl.add_factory< vital::algo::integrate_depth_maps,
    integrate_depth_maps >( "cuda" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.cuda" );
}

} // end namespace cuda

} // end namespace arrows

} // end namespace kwiver
