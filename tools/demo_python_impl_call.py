
from kwiver.vital import config
from kwiver.vital import plugin_management

def main_config_formatter_load_example():
    vpm = plugin_management.plugin_manager_instance()

    vpm.load_all_plugins()
    vpm.load_all_plugins() # we should be able to do this idempotently
    vpm.load_all_plugins()

    cb_new = config.empty_config()

    impl_names = vpm.impl_names_format_config_block()
    print("Impl names for format_config_block: ", impl_names)

    cb_new.set_value( "a", "1" )
    cb_new.set_value( "b", "2" )
    cb_new.set_value( "b:c", "other" )

main_config_formatter_load_example()
