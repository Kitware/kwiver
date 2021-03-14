#!/usr/bin/env python
#ckwg +28
# Copyright 2011-2020 by Kitware, Inc.
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


import unittest, pytest
from kwiver.sprokit.util.test import find_tests, run_test, test_error

class TestSprokitpymodules(unittest.TestCase):
    def test_load(self):
        from kwiver.vital.modules import modules
        from kwiver.sprokit.pipeline import process_factory
        modules.load_known_modules()
        types = process_factory.types()
        if ('test_python_process' not in types):
            test_error('Failed to load Python processes')

    def test_masking(self):
        from kwiver.vital.modules import modules
        from kwiver.sprokit.pipeline import process_factory
        modules.load_known_modules()
        types = process_factory.types()
        if ('test_python_process' in types):
            test_error('Failed to mask out Python processes')

    def test_extra_modules(self):
        from kwiver.vital.modules import modules
        from kwiver.sprokit.pipeline import process_factory
        modules.load_known_modules()
        types = process_factory.types()
        if ('extra_test_python_process' not in types):
            test_error('Failed to load extra Python processes')
