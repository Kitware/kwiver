#!/usr/bin/env python
# ckwg +28
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

class TestSprokitembedded_pipeline(unittest.TestCase):
    def test_create(self, cpp_pipeline_dir, py_pipeline_dir):
        from kwiver.sprokit.adapters import embedded_pipeline
        embedded_pipeline.EmbeddedPipeline()

    def test_api_calls(self, cpp_pipeline_dir, py_pipeline_dir):
        from kwiver.sprokit.adapters import embedded_pipeline
        from kwiver.sprokit.adapters import adapter_data_set
        pipeline_fname = 'simple_embedded_pipeline.pipe'
        path_to_pipe_file = os.path.join(cpp_pipeline_dir, pipeline_fname)
        ep = embedded_pipeline.EmbeddedPipeline()
        ep.build_pipeline(path_to_pipe_file)
        input_list = ep.input_port_names()
        if (len(input_list) != 3):
            test_error('input_port_list() returned list of length {}, expected {}'.format(len(input_list), 3))
        for port in input_list:
            print(('    ' + port))
        output_list = ep.output_port_names()
        if (len(output_list) != 3):
            test_error('output_port_list() returned list of length {}, expected {}'.format(len(output_list), 3))
        for port in output_list:
            print(('    ' + port))
        try:
            ep.wait()
        except RuntimeError:
            pass
        else:
            test_error('Calling wait() before start() should throw an error')
        ep.start()
        for i in range(10):
            ds = adapter_data_set.AdapterDataSet.create()
            for (val, port) in enumerate(input_list, start=i):
                ds[port] = val
            print('sending set:', i)
            ep.send(ds)
        print('Sending end of input element')
        ep.send_end_of_input()
        print('pipeline is full:', ep.full())
        print('pipeline is empty:', ep.empty())
        while True:
            ods = ep.receive()
            if ods.is_end_of_data():
                if (not ep.at_end()):
                    test_error('at_end() not set correctly')
                break
            for (port, d) in ods:
                print('   port:', port, ' value:', d.get_int())
        ep.wait()
        ep = embedded_pipeline.EmbeddedPipeline()
        ep.build_pipeline(path_to_pipe_file)
        ep.start()
        ds = adapter_data_set.AdapterDataSet.create()
        ep.send_end_of_input()
        ods = ep.receive()
        ep.stop()

    def run_roundtrip_pipeline(self, py_pipeline_dir, ads_in):
        from kwiver.vital.types import DetectedObject, DetectedObjectSet, BoundingBoxD
        from kwiver.sprokit.adapters import adapter_data_set, embedded_pipeline
        pipeline_fname = 'py_to_cpp_type_conversion.pipe'
        path_to_pipe_file = os.path.join(py_pipeline_dir, pipeline_fname)
        ep = embedded_pipeline.EmbeddedPipeline()
        ep.build_pipeline(path_to_pipe_file)
        ep.start()
        print('Sending ads:', ads_in)
        ep.send(ads_in)
        ep.send_end_of_input()
        while True:
            ods = ep.receive()
            if ods.is_end_of_data():
                break
            print('Recieved detected_object_set:', ods['detected_object_set'])
        ep.wait()

    def _create_detected_object_set(self):
        from kwiver.vital.types import DetectedObject, DetectedObjectSet, BoundingBoxD
        dos = DetectedObjectSet()
        bbox = BoundingBoxD(0, 10, 100, 50)
        dos.add(DetectedObject(bbox, 0.2))
        dos.add(DetectedObject(bbox, 0.5))
        dos.add(DetectedObject(bbox, 0.4))
        return dos

    def test_cpp_conversion(self, cpp_pipeline_dir, py_pipeline_dir):
        from kwiver.vital.types import DetectedObject, DetectedObjectSet, BoundingBoxD
        from kwiver.sprokit.adapters import adapter_data_set, embedded_pipeline
        from kwiver.sprokit.pipeline import datum
        ads_in = adapter_data_set.AdapterDataSet.create()
        ads_in['detected_object_set'] =self._create_detected_object_set()
        print('Starting roundtrip pipeline with a detected_object_set')
        self.run_roundtrip_pipeline(py_pipeline_dir, ads_in)
        ads_in = adapter_data_set.AdapterDataSet.create()
        ads_in['detected_object_set'] = datum.new(self._create_detected_object_set())
        print('Starting roundtrip pipeline with a datum containing a detected_object_set')
        self.run_roundtrip_pipeline(py_pipeline_dir, ads_in)

    def test_including_pipe_files(self, cpp_pipeline_dir, py_pipeline_dir):
        from kwiver.vital.types import (
            DetectedObject,
            DetectedObjectSet,
            BoundingBoxD,
        )
        from kwiver.sprokit.adapters import adapter_data_set, embedded_pipeline

        pipeline_fname = "test_includes.pipe"
        path_to_pipe_file = os.path.join(py_pipeline_dir, pipeline_fname)

        ep = embedded_pipeline.EmbeddedPipeline()

        # Note that we include an extra parameter here. This is where
        # to look for included pipe files
        ep.build_pipeline(path_to_pipe_file, py_pipeline_dir)

        ep.start()

        ads_in = adapter_data_set.AdapterDataSet.create()
        ads_in["port1"] = self._create_detected_object_set()
        print("Sending ads:", ads_in)
        ep.send(ads_in)
        ep.send_end_of_input()

        while True:
            ods = ep.receive()
            if ods.is_end_of_data():
                break
            print("Recieved detected_object_set:", ods["port1"])

        ep.wait()

