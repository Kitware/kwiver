"""
Scikit-build requires configuration to be passed to it's setup function.
Support for setup.cfg is forthcoming (as of skbuild v0.12
"""
from pathlib import Path
from setuptools import find_packages

from skbuild import setup


SCRIPT_DIR = Path(__file__).parent
PACKAGE_SRC = "python"
PACKAGE_NAME = "kwiver"


with open(SCRIPT_DIR / "VERSION", "r") as f:
    VERSION = f.read().strip()


with open(SCRIPT_DIR / "README.md", "r") as f:
    LONG_DESCRIPTION = f.read()


setup(
    # Basic Metadata ###########################################################
    name=PACKAGE_NAME,
    version=VERSION,
    description="Python and C++ toolkit that pulls together computer vision algorithms "
    " into highly modular run time configurable systems",
    long_description=LONG_DESCRIPTION,
    author="Kitware, Inc.",
    author_email="kwiver-developers@kitware.com",
    url="https://github.com/Kitware/kwiver",
    license="BSD 3-Clause",
    license_files=["LICENSE"],
    classifiers=[
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Operating System :: Unix",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
    ],
    platforms=[
        "linux",
        "Unix",
    ],
    # Options ##################################################################
    zip_safe=False,
    include_package_data=True,
    python_requires=">=3.6",
    # Package Specification ####################################################
    package_dir={"": PACKAGE_SRC},
    packages=find_packages(where=PACKAGE_SRC, include=[f"{PACKAGE_NAME}*"]),
    # Requirements #############################################################
    install_requires=[
        "numpy",
        "importlib-metadata>=3.7.0; python_version < '3.8'"
    ],
    # extras_require=[],
    # tests_require=[],
    # Entry-Points #############################################################
    entry_points={
        # 'kwiver.python_plugin_registration': [
        #     'pythread_process=kwiver.sprokit.schedulers.pythread_per_process',
        #     'apply_descriptor=kwiver.sprokit.processes.apply_descriptor',
        #     'process_image=kwiver.sprokit.processes.process_image',
        #     'print_number_process=kwiver.sprokit.processes.kw_print_number_process',
        #     'homography_writer=kwiver.sprokit.processes.homography_writer',
        #     'simple_homog_tracker=kwiver.sprokit.processes.simple_homog_tracker',
        #     'alexnet_descriptors=kwiver.sprokit.processes.pytorch.alexnet_descriptors',
        #     'resnet_augmentation=kwiver.sprokit.processes.pytorch.resnet_augmentation',
        #     'resnet_descriptors=kwiver.sprokit.processes.pytorch.resnet_descriptors',
        #     'srnn_tracker=kwiver.sprokit.processes.pytorch.srnn_tracker',
        #     'simple_object_detector=kwiver.vital.tests.alg.simple_image_object_detector'
        # ],
        # 'kwiver.cpp_search_paths': [
        #     'sprokit_process=kwiver.vital.util.entrypoint:sprokit_process_path',
        #     'applets=kwiver.vital.util.entrypoint:applets_path',
        #     'plugin_explorer=kwiver.vital.util.entrypoint:plugin_explorer_path'
        # ],
        # 'kwiver.env.ld_library_path': [
        #     'kwiver_ld_library_path=kwiver.vital.util.entrypoint:get_library_path',
        # ],
        # 'kwiver.env.logger_factory': [
        #     'vital_log4cplus_logger_factory=kwiver.vital.util.entrypoint:get_vital_logger_factory',
        # ],
        # 'console_scripts': [
        #     'plugin_explorer=kwiver.kwiver_tools:plugin_explorer',
        #     'kwiver=kwiver.kwiver_tools:kwiver',
        # ],
    },
    # Scikit-Build Stuff #######################################################
    cmake_minimum_required_version="3.15",  # matches primary CMakeLists.txt req
    cmake_source_dir=SCRIPT_DIR.as_posix(),
    # Where build libraries and such will be installed into in order to be
    # within the package module space.
    cmake_install_dir=f"./{PACKAGE_SRC}/{PACKAGE_NAME}",
    cmake_args=[
        "-DCMAKE_BUILD_TYPE=Release",
        "-DKWIVER_BUILD_SHARED=OFF",
        "-DKWIVER_ENABLE_PYTHON=ON",
        "-DKWIVER_PYTHON_MAJOR_VERSION=3",
        "-DPYBIND11_PYTHON_VERSION=3",
        "-DCMAKE_BUILD_WITH_INSTALL_RPATH=ON",
        "-DKWIVER_INSTALL_SET_UP_SCRIPT=OFF",
    ],
)
