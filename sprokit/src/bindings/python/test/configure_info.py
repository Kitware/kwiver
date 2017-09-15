# -*- coding: utf-8 -*-
#ckwg +28
# Copyright 2012-2013 by Kitware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
#  * Neither name of Kitware, Inc. nor the names of any contributors may be used
#    to endorse or promote products derived from this software without specific
#    prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""
This file is a configured resource, that is never symlinked even in developer
builds. Any changes should be made in the configure_info.py file in the source
repository, and then CMake should configure it into the build.
"""


__CMAKE_CONFIGURE_VARS__ = {
    'sprokit_test_pipelines_directory': '@sprokit_test_pipelines_directory@',
    'CMAKE_SOURCE_DIR': '@CMAKE_SOURCE_DIR@',
    'CMAKE_BINARY_DIR': '@CMAKE_BINARY_DIR@',
}


def keys():
    return __CMAKE_CONFIGURE_VARS__.keys()


def get(key):
    """
    Wraps __CMAKE_CONFIGURE_VARS__.__getitem__(key) with a check to ensure the
    file is configured
    """
    value = __CMAKE_CONFIGURE_VARS__[key]
    if value.startswith('@') and value.endswith('@'):
        raise Exception(
            'Attempting to get a configuration value from an unconfigured file'
        )
    return value
