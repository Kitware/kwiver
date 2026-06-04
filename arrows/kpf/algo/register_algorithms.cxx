// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/kpf/kwiver_algo_kpf_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <arrows/kpf/algo/detected_object_set_input_kpf.h>
#include <arrows/kpf/algo/detected_object_set_output_kpf.h>

namespace kwiver {

namespace arrows {

namespace kpf {

extern "C"
KWIVER_ALGO_KPF_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  static auto const module_name = std::string( "arrows_kpf" );

  using kvpf = ::kwiver::vital::plugin_factory;

  if( vpm.is_module_loaded( module_name ) )
  {
    return;
  }

  auto fact = vpm.add_factory< kwiver::vital::algo::detected_object_set_input,
    detected_object_set_input_kpf >( "kpf_input" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_kpf" );

  fact = vpm.add_factory< kwiver::vital::algo::detected_object_set_output,
    detected_object_set_output_kpf >( "kpf_output" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_kpf" );

  vpm.mark_module_as_loaded( module_name );
}

} // namespace kpf

} // namespace arrows

}     // end namespace
