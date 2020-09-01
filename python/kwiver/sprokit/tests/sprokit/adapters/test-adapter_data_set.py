#!/usr/bin/env python
# ckwg +28
# Copyright 2020 by Kitware, Inc.
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

from kwiver.sprokit.util.test import expect_exception, find_tests, run_test, test_error

from kwiver.sprokit.pipeline import datum

def test_import():
    try:
        import kwiver.sprokit.adapters.adapter_data_set
    except:
        test_error("Failed to import the adapter_data_set module")

def test_create():
    from kwiver.sprokit.adapters import adapter_data_set

    adapter_data_set.AdapterDataSet.create()
    adapter_data_set.AdapterDataSet.create(adapter_data_set.DataSetType.data)
    adapter_data_set.AdapterDataSet.create(adapter_data_set.DataSetType.end_of_input)

def test_add_get_datum():
    from kwiver.sprokit.adapters import adapter_data_set
    from kwiver.vital.types import Timestamp

    ads = adapter_data_set.AdapterDataSet.create()

    # Check that automatic conversion from py::object -> C++ int
    # and vice-versa works
    i = 5
    ads.add_value("int_port", i)
    if not ads.get_port_data("int_port") == i:
        test_error("Expected to get equivalent int back")

    # Check that automatic conversion from py::object -> C++ vital::timestamp
    # and vice-versa works
    t = Timestamp(10000, 10)
    ads.add_value("timestamp_port", t)
    if not ads.get_port_data("timestamp_port") == t:
        test_error("Expected to get equivalent timestamp back")

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        test_error("Expected two arguments")
        sys.exit(1)

    testname = sys.argv[1]

    run_test(testname, find_tests(locals()))
