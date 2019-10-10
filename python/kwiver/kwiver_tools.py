import os
import subprocess
import kwiver
import sys
from pkg_resources import iter_entry_points
from kwiver.vital import vital_logging

KWIVER_BIN_DIR = os.path.join(os.path.dirname(os.path.abspath(kwiver.__file__)), 'bin')
KWIVER_SUPPORTED_TOOLS = ["kwiver", "plugin_explorer", "pipeline_runner",
                          "pipe_config", "pipe_to_dot"]
logger = vital_logging.getLogger(__name__)

def _create_env_var_string( values ):
    env_var_value = ""
    # Append values
    for value in values:
        assert isinstance(value, str), "environment variables must be string, {0} specified".format( value )
        env_var_value += "{0}:".format(value)
    return env_var_value

def _setup_environment():
    # Add additional ld libraries
    ld_library_paths = []
    for entry_point in iter_entry_points('kwiver.env.ld_library_path'):
        ld_library_path = entry_point.load()()
        if not os.path.exists(ld_library_path):
            logger.warn("Invalid path {0} specified in {1}".format(ld_library_path, entry_point.name))
        else:
            ld_library_paths.append(ld_library_path)
    ld_library_path_str = _create_env_var_string(ld_library_paths)

    # Add logger factories
    vital_logger_factory = None
    for entry_point in iter_entry_points('kwiver.env.logger_factory', name='vital_log4cplus_logger_factory'):
        logger_factory = entry_point.load()()
        vital_logger_factory = logger_factory

    # Check if LD_LIBRARY_PATH is set to something and append it to the current ld library path
    if os.environ.get('LD_LIBRARY_PATH'):
        ld_library_path_str += os.environ.get('LD_LIBRARY_PATH')

    tool_environment = {
                            "LD_LIBRARY_PATH": ld_library_path_str,
                            "VITAL_LOGGER_FACTORY": vital_logger_factory,

                       }
    # Add the remaining environment variables without fiddling with what we have already set
    for env_var_name, env_var_val in os.environ.items():
        if env_var_name not in tool_environment.keys():
            tool_environment[env_var_name] = env_var_val

    return tool_environment


def _kwiver_tools(tool_name, args):
    assert tool_name in KWIVER_SUPPORTED_TOOLS, "Unsupported tool {0} specified".format(tool_name)
    tool_environment = _setup_environment()
    tool_path = os.path.join(KWIVER_BIN_DIR, tool_name)
    assert os.path.exists(tool_path), "Tool {0} not available in {1}".format(tool_name, tool_path)
    args.insert(0, tool_path)
    subprocess.run(args, shell=False, check=True, env=tool_environment)


def plugin_explorer():
    raise SystemExit(_kwiver_tools("plugin_explorer", sys.argv[1:]))


def kwiver():
    raise SystemExit(_kwiver_tools("kwiver", sys.argv[1:]))


def pipeline_runner():
    raise SystemExit(_kwiver_tools("pipeline_runner", sys.argv[1:]))


def pipe_to_dot():
    raise SystemExit(_kwiver_tools("pipe_to_dot", sys.argv[1:]))


def pipe_config():
    raise SystemExit(_kwiver_tools("pipe_config", sys.argv[1:]))
