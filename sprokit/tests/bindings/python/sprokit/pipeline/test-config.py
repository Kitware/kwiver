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
        import sprokit.pipeline.config  # NOQA
    except:
        raise AssertionError("Failed to import the config module")


def test_create():
    from sprokit.pipeline import config

    try:
        config.empty_config()
    except:
        raise AssertionError("Failed to create an empty configuration")

    config.ConfigKey()
    config.ConfigKeys()
    config.ConfigDescription()
    config.ConfigValue()


def test_api_calls():
    from sprokit.pipeline import config

    config.Config.block_sep
    config.Config.global_value


def test_has_value():
    from sprokit.pipeline import config

    c = config.empty_config()

    keya = 'keya'
    keyb = 'keyb'

    valuea = 'valuea'

    c.set_value(keya, valuea)

    if not c.has_value(keya):
        raise AssertionError("Block does not have value which was set")

    if c.has_value(keyb):
        raise AssertionError("Block has value which was not set")


def test_get_value():
    from sprokit.pipeline import config

    c = config.empty_config()

    keya = 'keya'

    valuea = 'valuea'

    c.set_value(keya, valuea)

    get_valuea = c.get_value(keya)

    if not valuea == get_valuea:
        raise AssertionError("Did not retrieve value that was set")


def test_get_value_nested():
    from sprokit.pipeline import config

    c = config.empty_config()

    keya = 'keya'
    keyb = 'keyb'

    valuea = 'valuea'

    c.set_value(keya + config.Config.block_sep + keyb, valuea)

    nc = c.subblock(keya)

    get_valuea = nc.get_value(keyb)

    if not valuea == get_valuea:
        raise AssertionError("Did not retrieve value that was set")


def test_get_value_no_exist():
    from sprokit.pipeline import config
    from sprokit.test.test import expect_exception

    c = config.empty_config()

    keya = 'keya'
    keyb = 'keyb'

    valueb = 'valueb'

    expect_exception('retrieving an unset value', BaseException,
                     c.get_value, keya)

    get_valueb = c.get_value(keyb, valueb)

    if not valueb == get_valueb:
        raise AssertionError("Did not retrieve default when requesting unset value")


def test_unset_value():
    from sprokit.pipeline import config
    from sprokit.test.test import expect_exception

    c = config.empty_config()

    keya = 'keya'
    keyb = 'keyb'

    valuea = 'valuea'
    valueb = 'valueb'

    c.set_value(keya, valuea)
    c.set_value(keyb, valueb)

    c.unset_value(keya)

    expect_exception('retrieving an unset value', BaseException,
                     c.get_value, keya)

    get_valueb = c.get_value(keyb)

    if not valueb == get_valueb:
        raise AssertionError("Did not retrieve value when requesting after an unrelated unset")


def test_available_values():
    from sprokit.pipeline import config

    c = config.empty_config()

    keya = 'keya'
    keyb = 'keyb'

    valuea = 'valuea'
    valueb = 'valueb'

    c.set_value(keya, valuea)
    c.set_value(keyb, valueb)

    avail = c.available_values()

    if not len(avail) == 2:
        raise AssertionError("Did not retrieve correct number of keys")

    try:
        for val in avail:
            pass
    except:
        raise AssertionError("Available values is not iterable")


def test_read_only():
    from sprokit.pipeline import config
    from sprokit.test.test import expect_exception

    c = config.empty_config()

    keya = 'keya'

    valuea = 'valuea'
    valueb = 'valueb'

    c.set_value(keya, valuea)

    c.mark_read_only(keya)

    expect_exception('setting a read only value', BaseException,
                     c.set_value, keya, valueb)

    get_valuea = c.get_value(keya)

    if not valuea == get_valuea:
        raise AssertionError("Read only value changed")


def test_read_only_unset():
    from sprokit.pipeline import config
    from sprokit.test.test import expect_exception

    c = config.empty_config()

    keya = 'keya'

    valuea = 'valuea'

    c.set_value(keya, valuea)

    c.mark_read_only(keya)

    expect_exception('unsetting a read only value', BaseException,
                     c.unset_value, keya)

    get_valuea = c.get_value(keya)

    if not valuea == get_valuea:
        raise AssertionError("Read only value was unset")


def test_subblock():
    from sprokit.pipeline import config

    c = config.empty_config()

    block1 = 'block1'
    block2 = 'block2'

    keya = 'keya'
    keyb = 'keyb'
    keyc = 'keyc'

    valuea = 'valuea'
    valueb = 'valueb'
    valuec = 'valuec'

    c.set_value(block1 + config.Config.block_sep + keya, valuea)
    c.set_value(block1 + config.Config.block_sep + keyb, valueb)
    c.set_value(block2 + config.Config.block_sep + keyc, valuec)

    d = c.subblock(block1)

    get_valuea = d.get_value(keya)

    if not valuea == get_valuea:
        raise AssertionError("Subblock does not inherit expected keys")

    get_valueb = d.get_value(keyb)

    if not valueb == get_valueb:
        raise AssertionError("Subblock does not inherit expected keys")

    if d.has_value(keyc):
        raise AssertionError("Subblock inherited unrelated key")


def test_subblock_view():
    from sprokit.pipeline import config

    c = config.empty_config()

    block1 = 'block1'
    block2 = 'block2'

    keya = 'keya'
    keyb = 'keyb'
    keyc = 'keyc'

    valuea = 'valuea'
    valueb = 'valueb'
    valuec = 'valuec'

    c.set_value(block1 + config.Config.block_sep + keya, valuea)
    c.set_value(block2 + config.Config.block_sep + keyb, valueb)

    d = c.subblock_view(block1)

    if not d.has_value(keya):
        raise AssertionError("Subblock does not inherit expected keys")

    if d.has_value(keyb):
        raise AssertionError("Subblock inherited unrelated key")

    c.set_value(block1 + config.Config.block_sep + keya, valueb)

    get_valuea1 = d.get_value(keya)

    if not valueb == get_valuea1:
        raise AssertionError("Subblock view persisted a changed value")

    d.set_value(keya, valuea)

    get_valuea2 = d.get_value(keya)

    if not valuea == get_valuea2:
        raise AssertionError("Subblock view set value was not changed in parent")


def test_merge_config():
    from sprokit.pipeline import config

    c = config.empty_config()
    d = config.empty_config()

    keya = 'keya'
    keyb = 'keyb'
    keyc = 'keyc'

    valuea = 'valuea'
    valueb = 'valueb'
    valuec = 'valuec'

    c.set_value(keya, valuea)
    c.set_value(keyb, valuea)

    d.set_value(keyb, valueb)
    d.set_value(keyc, valuec)

    c.merge_config(d)

    get_valuea = c.get_value(keya)

    if not valuea == get_valuea:
        raise AssertionError("Unmerged key changed")

    get_valueb = c.get_value(keyb)

    if not valueb == get_valueb:
        raise AssertionError("Conflicting key was not overwritten")

    get_valuec = c.get_value(keyc)

    if not valuec == get_valuec:
        raise AssertionError("New key did not appear")


def test_dict():
    from sprokit.pipeline import config
    from sprokit.test.test import expect_exception

    c = config.empty_config()

    key = 'key'
    value = 'oldvalue'

    if key in c:
        raise AssertionError("'%s' is in an empty config" % key)

    if c:
        raise AssertionError("An empty config is not falsy")

    c[key] = value

    if not c[key] == value:
        raise AssertionError("Value was not set")

    if key not in c:
        raise AssertionError("'%s' is not in config after insertion" % key)

    if not len(c) == 1:
        raise AssertionError("The len() operator is incorrect")

    if not c:
        raise AssertionError("A non-empty config is not truthy")

    value = 'newvalue'
    origvalue = 'newvalue'

    c[key] = value

    value = 'replacedvalue'

    if not c[key] == origvalue:
        raise AssertionError("Value was overwritten")

    del c[key]

    expect_exception('getting an unset value', BaseException,
                     c.__getitem__, key)

    expect_exception('deleting an unset value', BaseException,
                     c.__delitem__, key)

    value = 10

    c[key] = value

    if not c[key] == str(value):
        raise AssertionError("Value was not converted to a string")


if __name__ == '__main__':
    r"""
    CommandLine:
        python -m sprokit.tests.test-config
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
