"""
ckwg +31
Copyright 2017 by Kitware, Inc.
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

Test Python interface to vital::detected_object_set

"""

import nose.tools

from vital.types import DetectedObjectSet
from vital.types import DetectedObject
from vital.types import BoundingBox


class TestVitalDetectedObjectSet(object):

    def test_new(self):
        dset = DetectedObjectSet()
        sz = dset.size()
        nose.tools.assert_equal(sz, 0)

        # TBD test DetectedObjectSet(dobjs, count)

    def test_add(self):
        bbox = BoundingBox(10, 20, 30, 40)
        dobj = DetectedObject(bbox, 4.56)

        dset = DetectedObjectSet()
        dset.add(dobj)
        sz = dset.size()
        nose.tools.assert_equal(sz, 1)

    def test_select(self):
        dset = DetectedObjectSet()
        bbox = BoundingBox(10, 20, 30, 40)

        dobj = DetectedObject(bbox, 0.156)
        dset.add(dobj)

        dobj = DetectedObject(bbox, 0.256)
        dset.add(dobj)

        dobj = DetectedObject(bbox, 0.356)
        dset.add(dobj)

        dobj = DetectedObject(bbox, 0.456)
        dset.add(dobj)

        sel_set = dset.select(0.3)
        # TBD should select two items
