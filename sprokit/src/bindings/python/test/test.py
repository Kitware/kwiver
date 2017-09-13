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
Helpers for sprokit tests
"""
from os.path import dirname, exists, join
import pytest


def find_tests(scope):
    prefix = 'test_'
    tests = {}

    for name, obj in scope.items():
        if name.startswith(prefix) and callable(obj):
            tests[name[len(prefix):]] = obj

    return tests


def find_sprokit_test_pipelines_directory():
    """
    Uses a configured file to locate kwiver/sprokit/tests/data/pipelines
    """
    # Path to the file which will contain the directory name
    dir_location_fpath = join(dirname(__file__), 'pipeline_dir.txt')
    if not exists(dir_location_fpath):
        raise Exception('unable to find pipeline_dir.txt location')
    with open(dir_location_fpath, 'r') as file:
        pipeline_dir = file.read().strip()
    return pipeline_dir


def grab_test_pipeline_file(pipeline_fname):
    """ Gets a particular pipeline in the """
    pipeline_dir = find_sprokit_test_pipelines_directory()
    if not exists(pipeline_dir):
        import sys
        msg = (
            'Did the source dir move? '
            'pipeline_dir={} does not exist.'.format(pipeline_dir))
        if getattr(sys, '_called_from_test', False):
            pytest.skip(msg)
        else:
            raise AssertionError(msg)
    path = join(pipeline_dir, pipeline_fname)
    return path


def test_error(msg):
    """ Depricate in favor of python exceptions """
    import sys

    err_msg = "Error: %s\n" % msg
    sys.stderr.write(err_msg)
    raise AssertionError(err_msg)


def expect_exception(action, kind, func, *args, **kwargs):
    """ Depricate in favor of `pytest.raises` """
    got_exception = False

    try:
        func(*args, **kwargs)
    except kind:
        import sys

        t = sys.exc_info()[0]
        e = sys.exc_info()[1]

        sys.stderr.write("Got expected exception: %s: %s\n" % (str(t.__name__), str(e)))

        got_exception = True
    except BaseException:
        import sys
        import traceback

        t = sys.exc_info()[0]
        e = sys.exc_info()[1]
        bt = sys.exc_info()[2]
        bt_str = ''.join(traceback.format_tb(bt))

        raise AssertionError("Got unexpected exception: %s: %s:\n%s" % (str(t.__name__), str(e), bt_str))

        got_exception = True
    except:
        raise AssertionError("Got non-standard exception")

        got_exception = True

    if not got_exception:
        raise AssertionError("Did not get exception when %s" % action)
