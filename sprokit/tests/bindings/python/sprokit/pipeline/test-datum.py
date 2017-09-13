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
        import sprokit.pipeline.datum  # NOQA
    except:
        raise AssertionError("Failed to import the datum module")


def test_new():
    from sprokit.pipeline import datum

    d = datum.new('test_datum')

    if not d.type() == datum.DatumType.data:
        raise AssertionError("Datum type mismatch")

    if len(d.get_error()):
        raise AssertionError("A data datum has an error string")

    p = d.get_datum()

    if p is None:
        raise AssertionError("A data datum has None as its data")


def test_empty():
    from sprokit.pipeline import datum

    d = datum.empty()

    if not d.type() == datum.DatumType.empty:
        raise AssertionError("Datum type mismatch")

    if len(d.get_error()):
        raise AssertionError("An empty datum has an error string")

    p = d.get_datum()

    if p is not None:
        raise AssertionError("An empty datum does not have None as its data")


def test_flush():
    from sprokit.pipeline import datum

    d = datum.flush()

    if not d.type() == datum.DatumType.flush:
        raise AssertionError("Datum type mismatch")

    if len(d.get_error()):
        raise AssertionError("A flush datum has an error string")

    p = d.get_datum()

    if p is not None:
        raise AssertionError("A flush datum does not have None as its data")


def test_complete():
    from sprokit.pipeline import datum

    d = datum.complete()

    if not d.type() == datum.DatumType.complete:
        raise AssertionError("Datum type mismatch")

    if len(d.get_error()):
        raise AssertionError("A complete datum has an error string")

    p = d.get_datum()

    if p is not None:
        raise AssertionError("A complete datum does not have None as its data")


def test_error_():
    from sprokit.pipeline import datum

    err = 'An error'

    d = datum.error(err)

    if not d.type() == datum.DatumType.error:
        raise AssertionError("Datum type mismatch")

    if not d.get_error() == err:
        raise AssertionError("An error datum did not keep the message")

    p = d.get_datum()

    if p is not None:
        raise AssertionError("An error datum does not have None as its data")


if __name__ == '__main__':
    r"""
    CommandLine:
        python -m sprokit.tests.test-datum
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
