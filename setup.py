"""
Scikit-build requires configuration to be passed to it's setup function.
Support for setup.cfg is forthcoming (as of skbuild v0.12
"""

import os
from pathlib import Path
from setuptools import find_packages

from skbuild import setup


SCRIPT_DIR = Path(__file__).parent
PACKAGE_SRC = "python"
PACKAGE_NAME = "kwiver"


with open(SCRIPT_DIR / "VERSION.txt", "r") as f:
    VERSION_FROM_FILE = f.read().strip()


# This will generate versions like:
#   - On tags: "2.1.0"
#   - On main: "2.1.1.dev123+g7f3a2b1"
try:
    from setuptools_scm import get_version

    VERSION = get_version(
        root=SCRIPT_DIR,
        fallback_version=VERSION_FROM_FILE,
        local_scheme="node",
    )
except (ImportError, LookupError):
    # Not in a git repo or setuptools-scm not installed
    # Use VERSION.txt as-is
    VERSION = VERSION_FROM_FILE


with open(SCRIPT_DIR / "README.rst", "r") as f:
    LONG_DESCRIPTION = f.read()

# Set environment variable for CMake to pick up
# Adding f"-DKWIVER_VERSION={VERSION} to cmake_args didn't work
os.environ["KWIVER_WHEEL_VERSION"] = VERSION

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
        "Programming Language :: Python :: 3.8",
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
    python_requires=">=3.8",
    # Package Specification ####################################################
    package_dir={"": PACKAGE_SRC},
    packages=find_packages(
        where=PACKAGE_SRC,
        include=[f"{PACKAGE_NAME}*"],
        # xxx(python-arrows) bring back once arrows are adapted to new API
        exclude=[
            f"{PACKAGE_NAME}.arrows*",
            f"{PACKAGE_NAME}.sprokit*",
        ],
    ),
    # Requirements #############################################################
    install_requires=["numpy"],
    # extras_require=[],
    tests_require=["pytest"],
    # Entry-Points #############################################################
    entry_points={
        "kwiver.python_plugins": [
            "say=kwiver.vital.test_interface.python_say",
            "they_say=kwiver.vital.test_interface.python_they_say",
        ],
        "console_scripts": [
            "dump_klv=kwiver.tools.dump_klv:run",
        ],
    },
    # Scikit-Build Stuff #######################################################
    cmake_minimum_required_version="3.15",  # matches primary CMakeLists.txt req
    cmake_source_dir=SCRIPT_DIR.as_posix(),
    # Where build libraries and such will be installed into in order to be
    # within the package module space.
    cmake_install_dir=f"./{PACKAGE_SRC}/{PACKAGE_NAME}",
    cmake_args=[
        "-DKWIVER_ENABLE_PYTHON=ON",
        "-DKWIVER_PYTHON_MAJOR_VERSION=3",
        "-DPYBIND11_PYTHON_VERSION=3",
        "-DKWIVER_INSTALL_SET_UP_SCRIPT=OFF",
    ],
)
