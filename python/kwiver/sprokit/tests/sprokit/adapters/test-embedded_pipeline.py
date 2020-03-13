#!/usr/bin/env python
# ckwg +28
# Copyright 2011-2013 by Kitware, Inc.
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


def test_import(path_to_pipe_file):
    try:
        import kwiver.sprokit.adapters.embedded_pipeline
    except:
        test_error("Failed to import the embedded pipeline module")


def test_create(path_to_pipe_file):
    from kwiver.sprokit.adapters import embedded_pipeline

    embedded_pipeline.EmbeddedPipeline()


####send
####send_end_of_input
####receive
####full
####empty
####at_end
####start
####wait
#   stop
####input_port_names
def test_api_calls(path_to_pipe_file):
    from kwiver.vital.config import config
    from kwiver.vital.modules import modules
    from kwiver.sprokit.adapters import embedded_pipeline
    from kwiver.sprokit.pipeline import process
    from kwiver.sprokit.adapters import adapter_data_set

    #######
    # Run a very basic pipeline
    # pipe file and tests below are
    # based on embedded_pipeline tests
    # in basic_test.cxx in adapters/tests dir
    ep = embedded_pipeline.EmbeddedPipeline()
    ep.build_pipeline(path_to_pipe_file)

    # Check the input_ports
    input_list = ep.input_port_names()
    if len(input_list) != 3:
        test_error(
            "input_port_list() returned list of length {}, expected {}".format(
                len(input_list), 3
            )
        )
    for port in input_list:
        print("    " + port)

    # Check the output ports
    output_list = ep.output_port_names()
    if len(output_list) != 3:
        test_error(
            "output_port_list() returned list of length {}, expected {}".format(
                len(output_list), 3
            )
        )
    for port in output_list:
        print("    " + port)

    # Test that we can't call wait() yet
    try:
        ep.wait()
    except Exception:
        pass
    else:
        test_error("Calling wait() before start() should throw an error")

    ep.start()

    # Now send some data
    for i in range(10):
        ds = adapter_data_set.create()
        val = i

        for port in input_list:
            ds.add_int(port, val)
            val += 1

        print("sending set:", i)

        ep.send(ds)

    print("Sending end of input element")
    ep.send_end_of_input()

    print("pipeline is full:", ep.full())
    print("pipeline is empty:", ep.empty())
    while True:
        ods = ep.receive()
        if ods.is_end_of_data():
            if not ep.at_end():
                test_error("at_end() not set correctly")
            break
        for ix in ods:
            print("   port:", ix[0], " value:", ix[1].get_int())

    ep.wait()

    #######
    # Still need to test stop()
    # ep = embedded_pipeline.EmbeddedPipeline()
    # pipeline_fname = "simple_embedded_pipeline.pipe"
    # ep.build_pipeline(os.path.join(pipeline_dir, pipeline_fname))
    # ep.start()
    # ds = adapter_data_set.create()
    # ep.send_end_of_input()
    # ods = ep.receive()
    # ep.stop()


if __name__ == "__main__":
    import os
    import sys

    if not len(sys.argv) == 5:
        test_error("Expected three arguments")
        sys.exit(1)

    testname = sys.argv[1]

    os.chdir(sys.argv[2])

    sys.path.append(sys.argv[3])

    pipeline_dir = sys.argv[4]

    pipeline_fname = "simple_embedded_pipeline.pipe"

    path_to_pipe_file = os.path.join(pipeline_dir, pipeline_fname)

    from kwiver.sprokit.util.test import *

    run_test(testname, find_tests(locals()), path_to_pipe_file)
