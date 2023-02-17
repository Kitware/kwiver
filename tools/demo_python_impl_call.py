
from kwiver.vital import config
from kwiver.vital import plugin_management

def main_config_formatter_load_example():
    vpm = plugin_management.plugin_manager_instance()

    print(dir(plugin_management))

    vpm.load_all_plugins()

    cb_new = config.empty_config()

    cb_new.set_value( "a", "1" )
    cb_new.set_value( "b", "2" )
    cb_new.set_value( "b:c", "other" )

main_config_formatter_load_example()
