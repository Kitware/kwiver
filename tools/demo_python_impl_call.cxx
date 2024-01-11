#include <iostream>

#include <vital/config/config_block.h>
#include <vital/config/format_config_block.h>
#include <vital/logger/logger.h>
#include <vital/plugin_management/pluggable_macro_magic.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/test_interface/say.h>
#include <vital/test_interface/test_interface.h>

namespace kv = kwiver::vital;

static kv::logger_handle_t LOG = kv::get_logger( "kw-scratch" );

// ----------------------------------------------------------------------------

int
main_config_formatter_load_example()
{
  LOG_INFO( LOG, "Creating VPM" );

  auto& vpm = ::kv::plugin_manager::instance();

  vpm.load_all_plugins();
  vpm.load_all_plugins();  // we should be able to do this idempotently
  vpm.load_all_plugins();

  auto cb_empty = kv::config_block::empty_config();

  auto impl_names = vpm.impl_names< kv::format_config_block >();
  LOG_INFO( LOG, "What impls are there for format_config_block? "
                 "(found " << impl_names.size() << ")" );
  for( auto const& name : impl_names )
  {
    LOG_INFO( LOG, "  - " << name );
  }

  LOG_INFO( LOG, "Trying to instantiate a plugin" );

  auto impl_name = "markdown"; // "tree"
  auto fact =
    ::kv::implementation_factory_by_name< kv::format_config_block >();
  auto inst = fact.create( impl_name, cb_empty );

  LOG_INFO( LOG, kv::type_name( inst ) );
  LOG_INFO( LOG, "Inst is nullptr? " << inst );

  auto cb_new = kv::config_block::empty_config();
  cb_new->set_value( "a", "1" );
  cb_new->set_value( "b", "2" );
  cb_new->set_value( "b:c", "other" );

  inst->print( cb_new, std::cout );

  return 0;
}

// ----------------------------------------------------------------------------

void
main_say_example(std::string impl_name)
{
  auto& vpm = kv::plugin_manager::instance();
  vpm.load_all_plugins();

  auto impl_names = vpm.impl_names< kv::say >();
  if( impl_names.empty() )
  {
    LOG_INFO( LOG, "Found ZERO (0) `say` implementations." );
    return;
  }
  LOG_INFO( LOG, "Found implementations:" );
  for( auto const& name : impl_names )
  {
    LOG_INFO( LOG, "  - " << name );
  }

  // simulation configuration -- known to be empty for test interface impls.
  kv::config_block_sptr cb = kv::config_block::empty_config();

  // Create an implementation instance in the plugin-way -- via configuration.
  // Implementation handles currying config block into its own constructor.
  kv::say_sptr inst = kv::implementation_factory_by_name< kv::say >().create(
                        impl_name, cb );

  std::cout << "The implementation says:" << std::endl;
  std::cout << inst->says() << std::endl;
}

// ----------------------------------------------------------------------------

void
main_they_say_example()
{
  auto& vpm = kv::plugin_manager::instance();
  vpm.load_all_plugins();

  std::vector< std::string > they_names = { "cpp_they", "PythonTheyImpl" };
  std::vector< std::string > speaker_names = { "cpp", "PythonImpl" };

  std::cout << "Testing composite implementations" << std::endl;
  for ( auto t_name: they_names )
  {
    for ( auto s_name: speaker_names )
    {
      kv::config_block_sptr cb = kv::config_block::empty_config();
      cb->set_value( "speaker", s_name );

      kv::say_sptr inst = kv::implementation_factory_by_name< kv::say >().create(
                            t_name, cb );

      std::cout << inst->says() << std::endl;
    }
  }
  std::cout << std::endl;
}

// ----------------------------------------------------------------------------

void
main_macro_magic()
{
  namespace kw = ::kwiver::vital;

  kv::config_block_sptr cb = kv::config_block::empty_config();
  kv::test_impl_parameterized::get_default_config( *cb );

  cb->set_value( "a", 7 );
  cb->set_value( "b", "bar" );

  kv::test_interface_sptr i =
    std::dynamic_pointer_cast< kv::test_interface >(
      kv::test_impl_parameterized::from_config( cb )
      );

  std::cout << i->test() << std::endl;

  std::shared_ptr<kv::test_impl_parameterized> ip =
      std::dynamic_pointer_cast<kv::test_impl_parameterized>(i);
  std::cout << "A value is: " << ip->get_a() << std::endl;
}

// ----------------------------------------------------------------------------

int
main()
{
//  main_config_formatter_load_example();

  std::cout << std::endl;

  std::vector< std::string > impl_names = {"cpp", "PythonImpl"};

  for ( auto name: impl_names )
  {
    std::cout << "Testing say implementation: " << name << std::endl;
    main_say_example(name);
    std::cout << std::endl;
  }

  main_they_say_example();

  main_macro_magic();

  return 0;
}
