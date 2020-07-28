"""
ckwg +31
Copyright 2016 by Kitware, Inc.
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

Tests for TrackState interface class

"""
import unittest

import nose.tools
import numpy

from kwiver.vital.types import TrackState, Feature, Descriptor


# kwiver::vital::track_state doesn't have features or descriptors
class TestTrackState (unittest.TestCase):

    def test_new_ts(self):
        TrackState(0)
        TrackState(23456)

        # With feat, desc, feat/desc
        #f = Feature()
        #d = Descriptor()
        #TrackState(0, feature=f)
        #TrackState(0, descriptor=d)
        #TrackState(0, f, d)


    def test_frame_id(self):
        ts = TrackState(0)
        nose.tools.assert_equal(ts.frame_id, 0)

        ts = TrackState(14691234578)
        nose.tools.assert_equal(ts.frame_id, 14691234578)

'''
    def test_feat_empty(self):
        ts = TrackState(0)
        nose.tools.assert_is_none(ts.feature)

    def test_desc_empty(self):
        ts = TrackState(0)
        nose.tools.assert_is_none(ts.descriptor)

    def test_feat(self):
        f = Feature()
        ts = TrackState(0, f)
        nose.tools.assert_equal(ts.feature, f)

    def test_desc(self):
        # Some random initialized descriptor
        d = Descriptor()
        d[:] = 1
        nose.tools.assert_equal(d.sum(), d.size)

        ts = TrackState(0, descriptor=d)
        numpy.testing.assert_equal(d, ts.descriptor)
'''
