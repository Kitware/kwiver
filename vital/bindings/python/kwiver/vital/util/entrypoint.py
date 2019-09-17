import kwiver
import os

def get_library_path():
    return os.path.join(os.path.dirname(os.path.abspath(kwiver.__file__)), 'lib')

def get_vital_logger_factory():
    return os.path.join(os.path.dirname(os.path.abspath(kwiver.__file__)), 'lib', 'kwiver', 'modules', 'vital_log4cplus_logger')

