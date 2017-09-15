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

TODO:
    This module should probably be renamed to something that isn't the package
    name. The package should probably be renamed too.  E.G. instead of
    sprokit.test.test, maybe sprokit.test.helpers or sprokit.context.test or
    sprokit.util.test... not sure, I'm open to idea.
"""
import sys
from os.path import exists, join
import pytest
import inspect
from six.moves import range


def test_module(module_name=None, module_fpath=None, module_globals=None):
    """
    Helper script for the __main__ of each test, that tests it with py.test

    All arguments are optional and will be dynamically inferred if not
    specified.

    Notes:
        When put in the __main__ part of a module, this allows for all tests in
        that module to be run via: `python -c modname [*args...]`, where args
        are passed to py.test. OR just a single test can be run via
        `python -c modname <funcname> [*args...]`

    Args:
        module_fpath (str): the __file__ of the module (inferred if None)
        module_name (str): the __name__ of the module (inferred if None)
    """
    if module_name is None:
        # get the parent frame to lookup caller module name and its globals
        frame = get_stack_frame(n=1)
        module_globals = frame.f_globals
        module_name = module_globals['__name__']
    if module_globals is None:
        # using the test module name (typically __main__)
        module = sys.modules[module_name]
        module_globals = module.__dict__
    if module_fpath is None:
        module_fpath = module_globals['__file__']
        if module_fpath.endswith('.pyc'):
            module_fpath = module_fpath[:-1]

    argv = list(sys.argv[1:])
    if len(argv) > 0:
        # split argv[0] in case it is paramatarized (see test-run.py)
        target_func = argv[0].split('[')[0]
    else:
        target_func = None

    argv = list(sys.argv[1:])
    if target_func is not None and target_func in module_globals:
        # If arg[0] is a function in this file put it in pytest format
        # FIXME: this scheme will not work for class-based tests, we will
        # either need to switch to py.test syntax (i.e. :: to separate nodes)
        # or use the testname parser (in kwiver/CMake/tools) to known which
        # function names are valid. (We could also use heuristics like not
        # argv[0].startswith('-'))
        argv[0] = module_fpath + '::' + argv[0]
        argv.append('-s')  # dont capture stdout for single tests
    else:
        # Test everything in this module (sending all extra args to py.test)
        argv.insert(0, module_fpath)
        # Force pytest to run in forked mode so test-pymodules.py passes
        # References:
        #     https://stackoverflow.com/questions/11802316/nose2-vs-py-test-with-isolated-processes
        #     https://stackoverflow.com/questions/34644252/testing-incompatible-configs-with-py-test
        #     https://github.com/pytest-dev/pytest-xdist#installation
        # pip install pytest-xdist

        if '--forked' not in argv:
            argv.append('--forked')

    CHECK_FOR_FORKED = False
    if not CHECK_FOR_FORKED:
        # Otherwise if we dont care about checking for the forked extension and
        # printing a message then the alternative code can be removed and we
        # can simply use this one-liner. Should be taken care of
        # requirements.txt anyway
        pytest.main(argv)
    if CHECK_FOR_FORKED:
        # If we want to ensure the forked extension exists and print out a
        # meaningful message, we have to go through all this junk
        class SysExitException(Exception):
            def __init__(self, *args):
                self.args = args

        def _hack_exit(*args):
            raise SysExitException(*args)
        # Hack, don't let pytest without coming back to us
        _real_exit = sys.exit
        sys.exit = _hack_exit

        # Execute pytest
        try:
            retcode = pytest.main(argv)
            print('retcode = {!r}'.format(retcode))
        except SysExitException as sys_ex:
            try:
                import pytest_forked  # NOQA
            except ImportError as ex:
                raise ImportError(
                    'must install pytest-xdist via "pip install pytest-xdist"\n' +
                    str(ex)
                )
            print('sprokit test error {}'.format(sys_ex.args))
            _real_exit(*sys_ex.args)
        finally:
            sys.exit = _real_exit


def get_stack_frame(n=0, strict=True):
    """
    Gets the current stack frame or any of its ancestors using dynamic analysis

    Args:
        n (int): n=0 means the frame you called this function in.
                 n=1 is the parent frame.
        strict (bool): (default = True)

    Returns:
        frame: frame_cur

    Example:
        >>> from sprokit.test.test import *  # NOQA
        >>> frame_cur = get_stack_frame(n=0)
        >>> print('frame_cur = %r' % (frame_cur,))
        >>> assert frame_cur.f_globals['frame_cur'] is frame_cur
    """
    frame_cur = inspect.currentframe()
    # Use n+1 to always skip the frame of this function
    for ix in range(n + 1):
        frame_next = frame_cur.f_back
        if frame_next is None:  # nocover
            if strict:
                raise AssertionError('Frame level %r is root' % ix)
            else:
                break
        frame_cur = frame_next
    return frame_cur


def find_tests(scope):
    prefix = 'test_'
    tests = {}

    for name, obj in scope.items():
        if name.startswith(prefix) and callable(obj):
            tests[name[len(prefix):]] = obj

    return tests


def grab_test_pipeline_file(pipeline_fname):
    """ Gets a particular pipeline in the """
    from sprokit.test import configure_info
    pipeline_dir = configure_info.get('sprokit_test_pipelines_directory')
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
