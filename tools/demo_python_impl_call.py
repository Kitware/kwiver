
from kwiver.vital import config
from kwiver.vital import plugin_management

print(dir(plugin_management))

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

def main_say_example():
    vpm = plugin_management.plugin_manager_instance()
    vpm.load_all_plugins()

    impl_names = vpm.impl_names_say()
    print("Impl names for say: ", impl_names)

    cb = config.empty_config()
    sf = plugin_management.SayFactory()

    for impl_name in impl_names:
        impl = sf.create( impl_name, cb )
        print( "The", impl_name, "implementation says:")
        print( impl.says() )
        print()

main_config_formatter_load_example()
main_say_example()
