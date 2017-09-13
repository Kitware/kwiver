#!/usr/bin/env python
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

try:
    from sprokit.test import test
except ImportError:
    print('Could not load sprokit test helper module')
    pass


def test_error_string():
    try:
        test.test_error('an error')
    except Exception as ex:
        pass
    else:
        raise AssertionError('Should have raised an exception')


def test_error_string_stdout():
    import sys

    sys.stdout.write('Error: an error\n')


def test_error_string_second_line():
    """
    CommandLine:
        ctest -R error_string_second_line
    """
    import sys

    sys.stderr.write('Not an error\n')
    try:
        test.test_error("an error")
    except Exception as ex:
        pass
    # else:
    #     raise AssertionError('Error: should have raised an error')


def raise_exception():
    raise NotImplementedError


def test_expected_exception():
    test.expect_exception('when throwing an exception', NotImplementedError,
                           raise_exception)
    print('correctly handled exception')


def test_unexpected_exception():
    # TODO: can likely use pytest instead
    try:
        test.expect_exception('when throwing an unexpected exception', SyntaxError,
                              raise_exception)
    except AssertionError:
        print('correctly handled exception')
    else:
        raise AssertionError('Should have raised an exception')


if __name__ == '__main__':
    r"""
    CommandLine:
        python -m sprokit.tests.test-test -s --verbose
    """
    import sys
    import pytest
    argv = list(sys.argv[1:])
    if len(argv) > 0 and argv[0] in vars():
        # If arg[0] is a function in this file put it in pytest format
        argv[0] = __file__ + '::' + argv[0]
        argv.append('-s')  # dont capture stdout for single tests
    else:
        # ensure args refer to this file
        argv.insert(0, __file__)
    pytest.main(argv)
