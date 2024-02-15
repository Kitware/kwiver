#include <iostream>

#include <vital/config/config_block.h>
#include <vital/config/format_config_block.h>
#include <vital/logger/logger.h>
#include <vital/plugin_management/pluggable_macro_magic.h>
#include <vital/plugin_management/plugin_manager.h>

namespace kv = kwiver::vital;

static kv::logger_handle_t LOG = kv::get_logger( "kw-scratch" );

// ----------------------------------------------------------------------------

int
main_config_formatter_load_example()
{
  LOG_INFO( LOG, "Creating VPM" );

  auto& vpm = ::kv::plugin_manager::instance();

  vpm.load_all_plugins();

  auto cb_empty = kv::config_block::empty_config();
  cb_empty->set_value( "opt_prefix", ">>" );

  auto impl_names = vpm.impl_names< kv::format_config_block >();
  LOG_INFO( LOG, "What impls are there for format_config_block? "
                 "(found " << impl_names.size() << ")" );

  auto cb_new = kv::config_block::empty_config();
  cb_new->set_value( "a", "1" );
  cb_new->set_value( "b", "2" );
  cb_new->set_value( "b:c", "other" );

  auto fact =
    ::kv::implementation_factory_by_name< kv::format_config_block >();

  for( auto const& name : impl_names )
  {
    std::cout << "Format config for " << name << " implementation" << std::endl;

    auto inst = fact.create( name, cb_empty );

    inst->print( cb_new, std::cout );
    std::cout << std::endl;
  }

  return 0;
}

// ----------------------------------------------------------------------------

int
main()
{
  main_config_formatter_load_example();

  return 0;
}
