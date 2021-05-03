"""
ckwg +31
Copyright 2020 by Kitware, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Kitware, Inc. nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific
   prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

==============================================================================

Tests for the vital classes metadata_map

"""


import numpy as np
import unittest, pytest
from kwiver.vital.tests.cpp_helpers import metadata_map_helpers as mmh
from kwiver.vital.tests.py_helpers import no_call_pure_virtual_method
from kwiver.vital.types import(
    MetadataMap,
    SimpleMetadataMap,
    Metadata,
    metadata_tags as mt,
)
from kwiver.vital.types.metadata_traits import *
from kwiver.vital.types.metadata import *

class TestMetadataMap(unittest.TestCase):
    def test_init(self):
        MetadataMap()
    def test_no_call_pure_virt(self):
        m = MetadataMap()
        no_call_pure_virtual_method(m.size)
        no_call_pure_virtual_method(m.metadata)
        no_call_pure_virtual_method(m.has_item,
                                    mt.tags.VITAL_META_UNIX_TIMESTAMP, 1 )
        no_call_pure_virtual_method(m.get_item,
                                    mt.tags.VITAL_META_UNIX_TIMESTAMP, 1 )
        no_call_pure_virtual_method(m.get_vector, 1)
        no_call_pure_virtual_method(m.frames)

class MetadataMapSub(MetadataMap):
    def __init__(self, data):
        MetadataMap.__init__(self)
        self.data_ = data

    def size(self):
        return len(self.data_)

    def metadata(self):
        return self.data_

    def has_item(self, tag, fid):
        return True

    def get_item(self, tag, fid):
        return self.data_[1][0].find(tag)

    def get_vector(self, fid):
        return []

    def frames(self):
        return set(self.data_.keys())

class TestMetadataMapSub(unittest.TestCase):
    def populate_metadata(self, m):
        m.add(100, self.tags[0])
        m.add("hello", self.tags[3])
        m.add(2, self.tags[1])
        m.add(True, self.tags[4])
        m.add(1.1, self.tags[2])

    @classmethod
    def setUp(self):
        self.tags = [
            mt.tags.VITAL_META_UNKNOWN,
            mt.tags.VITAL_META_UNIX_TIMESTAMP,
            mt.tags.VITAL_META_SLANT_RANGE,
            mt.tags.VITAL_META_MISSION_ID,
            mt.tags.VITAL_META_VIDEO_KEY_FRAME,
        ]
        meta = Metadata()
        self.populate_metadata(self, meta)
        self.small_items = np.array([meta])
        self.map = dict(zip([1],[self.small_items]))

    def test_init(self):
        MetadataMapSub(self.map)

    def test_inheritance(self):
        assert(issubclass(MetadataMapSub, MetadataMap))

    def test_size(self):
        mms = MetadataMapSub(self.map)
        self.assertEqual(mmh.size(mms), 1)

    def test_metadata(self):
        mms = MetadataMapSub(self.map)
        ret_val = mmh.metadata(mms)
        np.testing.assert_array_equal(self.map[1], ret_val[1])

    def test_has_get_item(self):
        mms = MetadataMapSub(self.map)
        assert(mmh.has_item(mms, self.tags[1], 1))
        ret_typed = mms.get_item(self.tags[0], 1)
        self.assertEqual(ret_typed.name, "Unknown / Undefined entry")

    def test_get_vector(self):
        mms = MetadataMapSub(self.map)
        self.assertListEqual(mmh.get_vector(mms, 1), [])

    def test_frames(self):
        mms = MetadataMapSub(self.map)
        ret_set = mmh.frames(mms)
        self.assertIn(1, ret_set)
        self.assertEqual(len(ret_set), 1)


class TestSimpleMetadataMap(unittest.TestCase):
    @classmethod
    def setUp(self):
        self.small_tag = [
            mt.tags.VITAL_META_UNKNOWN,
            mt.tags.VITAL_META_UNIX_TIMESTAMP,
            mt.tags.VITAL_META_SLANT_RANGE,
            mt.tags.VITAL_META_MISSION_ID,
            mt.tags.VITAL_META_VIDEO_KEY_FRAME,
        ]

    def test_init(self):
        SimpleMetadataMap()

    def test_size(self):
        sm = SimpleMetadataMap()
        self.assertEqual(sm.size(), 0)

    def test_metadata(self):
        sm = SimpleMetadataMap()
        m = sm.metadata()
        assert(isinstance(m, dict))

    def test_has_get_item(self):
        sm = SimpleMetadataMap()
        self.assertFalse(sm.has_item(self.small_tag[1], 1))
        with self.assertRaisesRegex(
            RuntimeError, "Metadata map does not contain frame 1",
        ):
            sm.get_item(self.small_tag[1], 1)

    def test_get_vector(self):
        sm = SimpleMetadataMap()
        vm = sm.get_vector(1)
        assert(isinstance(vm, list))
        self.assertListEqual(vm, [])

    def test_frames(self):
        sm = SimpleMetadataMap()
        frames = sm.frames()
        assert(isinstance(frames, set))
        self.assertSetEqual(frames, set())
