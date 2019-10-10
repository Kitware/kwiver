from pkg_resources import iter_entry_points
from kwiver.vital import vital_logging
from kwiver import PYTHON_PLUGIN_ENTRYPOINT, CPP_SEARCH_PATHS_ENTRYPOINT

import os

logger = vital_logging.getLogger(__name__)

def get_python_plugins_from_entrypoint():
    """
    Get a list of python plugins that were registered through
    kwiver.python_plugin_registration
    :return: A list of zero or more python modules containing registration
             functions
    """
    py_modules = []
    for entry_point in iter_entry_points(PYTHON_PLUGIN_ENTRYPOINT):
        py_modules.append(entry_point.load())
    return py_modules


def get_cpp_paths_from_entrypoint():
    """
    Get a list of paths that were advertised through kwiver.cpp_search_paths
    :return: A list of paths for c++ plugins
    """
    additional_search_paths = []
    for entry_point in iter_entry_points(CPP_SEARCH_PATHS_ENTRYPOINT):
        search_path = entry_point.load()()
        if os.path.exists(search_path):
            additional_search_paths.append(search_path)
        else:
            logger.warn('Invalid search path {0} specified by {1}'.format(search_path,
                        entry_point.key()))
    return additional_search_paths
