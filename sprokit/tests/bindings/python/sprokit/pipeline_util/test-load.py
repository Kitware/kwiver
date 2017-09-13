#!/usr/bin/env python
#ckwg +28
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


def test_import():
    try:
        import sprokit.pipeline_util.load  # NOQA
    except:
        raise AssertionError("Failed to import the load module")


def test_create():
    from sprokit.pipeline_util import load

    load.Token()
    load.ConfigFlag()
    load.ConfigFlags()
    load.ConfigValue()
    load.ConfigValues()
    load.ConfigBlock()
    load.ProcessBlock()
    load.ConnectBlock()
    load.PipeBlock()
    load.PipeBlocks()
    load.ClusterConfig()
    load.ClusterInput()
    load.ClusterOutput()
    load.ClusterSubblock()
    load.ClusterSubblocks()
    load.ClusterBlock()
    load.ClusterDefineBlock()
    load.ClusterDefineBlocks()


def test_api_calls():
    from sprokit.pipeline import config
    from sprokit.pipeline import process
    from sprokit.pipeline import process_factory
    from sprokit.pipeline_util import load

    o = load.ConfigValue()
    o.key
    o.value
    o.value = config.ConfigValue()

    o = load.ConfigBlock()
    o.key
    o.values
    o.values = load.ConfigValues()

    o = load.ProcessBlock()
    o.name
    o.type
    o.config_values
    o.name = process.ProcessName()
    o.type = process.ProcessType()
    o.config_values = load.ConfigValues()

    o = load.ConnectBlock()
    o.from_
    o.to
    o.from_ = process.PortAddr()
    o.to = process.PortAddr()

    o = load.PipeBlock()
    o.config = load.ConfigBlock()
    o.config
    o.process = load.ProcessBlock()
    o.process
    o.connect = load.ConnectBlock()
    o.connect

    o = load.ClusterConfig()
    o.description
    o.config_value
    o.description = config.ConfigDescription()
    o.config_value = load.ConfigValue()

    o = load.ClusterInput()
    o.description
    o.from_
    o.targets
    o.description = process.PortDescription()
    o.from_ = process.Port()
    o.targets = process.PortAddrs()

    o = load.ClusterOutput()
    o.description
    o.from_
    o.to
    o.description = process.PortDescription()
    o.from_ = process.PortAddr()
    o.to = process.Port()

    o = load.ClusterSubblock()
    o.config = load.ClusterConfig()
    if o.config is None:
        raise AssertionError("The 'config' is None when the cluster subblock is a config")
    if o.input is not None:
        raise AssertionError("The 'input' is not None when the cluster subblock is a config")
    if o.output is not None:
        raise AssertionError("The 'output' is not None when the cluster subblock is a config")
    o.input = load.ClusterInput()
    if o.config is not None:
        raise AssertionError("The 'config' is not None when the cluster subblock is an input")
    if o.input is None:
        raise AssertionError("The 'input' is None when the cluster subblock is an input")
    if o.output is not None:
        raise AssertionError("The 'output' is not None when the cluster subblock is an input")
    o.output = load.ClusterOutput()
    if o.config is not None:
        raise AssertionError("The 'config' is not None when the cluster subblock is an output")
    if o.input is not None:
        raise AssertionError("The 'input' is not None when the cluster subblock is an output")
    if o.output is None:
        raise AssertionError("The 'output' is None when the cluster subblock is an output")

    o = load.ClusterBlock()
    o.type
    o.description
    o.subblocks
    o.type = process.ProcessType()
    o.description = process_factory.ProcessDescription()
    o.subblocks = load.ClusterSubblocks()

    o = load.ClusterDefineBlock()
    o.config = load.ConfigBlock()
    if o.config is None:
        raise AssertionError("The 'config' is None when the pipe subblock is a config")
    if o.process is not None:
        raise AssertionError("The 'process' is not None when the pipe subblock is a config")
    if o.connect is not None:
        raise AssertionError("The 'connect' is not None when the pipe subblock is a config")
    if o.cluster is not None:
        raise AssertionError("The 'cluster' is not None when the pipe subblock is a config")
    o.process = load.ProcessBlock()
    if o.config is not None:
        raise AssertionError("The 'config' is not None when the pipe subblock is a process")
    if o.process is None:
        raise AssertionError("The 'process' is None when the pipe subblock is a process")
    if o.connect is not None:
        raise AssertionError("The 'connect' is not None when the pipe subblock is a process")
    if o.cluster is not None:
        raise AssertionError("The 'cluster' is not None when the pipe subblock is a process")
    o.connect = load.ConnectBlock()
    if o.config is not None:
        raise AssertionError("The 'config' is not None when the pipe subblock is a connection")
    if o.process is not None:
        raise AssertionError("The 'process' is not None when the pipe subblock is a connection")
    if o.connect is None:
        raise AssertionError("The 'connect' is None when the pipe subblock is a connection")
    if o.cluster is not None:
        raise AssertionError("The 'cluster' is not None when the pipe subblock is a connection")
    o.cluster = load.ClusterBlock()
    if o.config is not None:
        raise AssertionError("The 'config' is not None when the pipe subblock is a cluster")
    if o.process is not None:
        raise AssertionError("The 'process' is not None when the pipe subblock is a cluster")
    if o.connect is not None:
        raise AssertionError("The 'connect' is not None when the pipe subblock is a cluster")
    if o.cluster is None:
        raise AssertionError("The 'cluster' is None when the pipe subblock is a cluster")


def test_simple_pipeline():
    from sprokit.test import test
    path = test.grab_test_pipeline_file('simple_pipeline.pipe')

    from sprokit.pipeline_util import load

    blocks = load.load_pipe_file(path)
    with open(path, 'r') as fin:
        load.load_pipe(fin)


def test_cluster_multiplier():
    from sprokit.test import test
    path = test.grab_test_pipeline_file('cluster_multiplier.pipe')

    from sprokit.pipeline_util import load

    blocks = load.load_cluster_file(path)
    with open(path, 'r') as fin:
        load.load_cluster(fin)


if __name__ == '__main__':
    r"""
    CommandLine:
        python -m sprokit.tests.test-load
    """
    import pytest
    import sys
    argv = list(sys.argv[1:])
    if len(argv) > 0 and argv[0] in vars():
        # If arg[0] is a function in this file put it in pytest format
        argv[0] = __file__ + '::' + argv[0]
        argv.append('-s')  # dont capture stdout for single tests
    else:
        # ensure args refer to this file
        argv.insert(0, __file__)
    pytest.main(argv)
