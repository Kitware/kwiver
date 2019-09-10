from skbuild import setup
from setuptools import find_packages
import os.path as osp
import os

kwiver_root = '../../../'
kwiver_install_dir = 'kwiver'
fletch_root = '' #TODO: Make this command line parameter
setup(
        name='kwiver',
        version='1.4.0',
        author='Kitware',
        author_email='http://public.kitware.com/mailman/listinfo/kwiver-users',
        cmake_install_dir=kwiver_install_dir,
        cmake_source_dir=kwiver_root,
        cmake_minimum_required_version='3.3',
        packages=[ 'kwiver',
                   'kwiver.vital',
                   'kwiver.vital.types',
                   'kwiver.vital.modules',
                   'kwiver.vital.algo',
                   'kwiver.vital.exceptions' ],
        setup_requires=[
                        'cmake',
                        'scikit-build'
                       ],
        install_requires=[
                          'numpy',
                          'pillow',
                          'six',
                         ],
        cmake_args=[
                    '-DCMAKE_BUILD_TYPE=Release',
                    '-DKWIVER_BUILD_SHARED=OFF',
                    '-DKWIVER_ENABLE_C_BINDINGS=ON',
                    '-DKWIVER_ENABLE_PYTHON=ON',
                    '-DKWIVER_PYTHON_MAJOR_VERSION=3',
                    '-DPYBIND11_PYTHON_VERSION=3',
                    '-DCMAKE_BUILD_WITH_INSTALL_RPATH=ON',
                    '-DKWIVER_ENABLE_TOOLS=ON',
                    '-Dfletch_DIR={0}'.format(fletch_root)
                   ],
        entry_points={
            # Module containing python plugins
            'kwiver.python_plugin_registration': [
                ],
            'kwiver.cpp_search_paths': [
                ],
        },
    )
