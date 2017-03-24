"""
ckwg +31
Copyright 2015-2016 by Kitware, Inc.
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

Test Python interface to vital::detected_object

"""
# -*- coding: utf-8 -*-
import nose.tools

from vital.types import DetectedObject
from vital.types import BoundingBox
from vital.types import DetectedObjectType
from vital.types import ImageContainer
from vital.types import Image


class TestVitalDetectedObject (object):

    def test_new(self):
        bbox = BoundingBox(10, 20, 30, 40)
        dobj = DetectedObject(bbox, 4.56)

        # test with type
        dot = DetectedObjectType()
        dot.set_score("class_name", 0.345)
        dobj = DetectedObject(bbox, 4.56, dot)

    def test_bounding_box(self):
        bbox = BoundingBox(10, 20, 30, 40)
        dobj = DetectedObject(bbox, 4.56)

        bbox2 = dobj.bounding_box()
        nose.tools.assert_equal(bbox, bbox2)
        bbox = BoundingBox(11, 22, 33, 44)
        dobj.set_bounding_box(bbox)

        bbox2 = dobj.bounding_box()
        nose.tools.assert_equal(bbox, bbox2)

    def test_confidence(self):
        bbox = BoundingBox(10, 20, 30, 40)
        dobj = DetectedObject(bbox, 4.56)

        fid = dobj.confidence()
        nose.tools.assert_equal(fid, 4.56)

        dobj.set_confidence(.5678)
        fid = dobj.confidence()
        nose.tools.assert_equal(fid, .5678)

    def test_type(self):
        bbox = BoundingBox(10, 20, 30, 40)
        dobj = DetectedObject(bbox, 4.56)

        dot = DetectedObjectType()
        dot.set_score("class_name", 0.345)

        dobj.set_type(dot)
        # TBD test type object

    def test_index(self):
        bbox = BoundingBox(10, 20, 30, 40)
        dobj = DetectedObject(bbox, 4.56)

        dobj.set_index(123)
        idx = dobj.index()
        nose.tools.assert_equal(idx, 234)

    def test_name(self):
        bbox = BoundingBox(10, 20, 30, 40)
        dobj = DetectedObject(bbox, 4.56)

        dobj.set_name("best-detector")
        det_name = dobj.name()
        nose.tools.assert_equal(det_name, "best-detector")

    def test_mask(self):
        bbox = BoundingBox(10, 20, 30, 40)
        dobj = DetectedObject(bbox, 4.56)

        img = Image()
        imgc = ImageContainer(img)

        dobj.set_mask(imgc)
        # TBD test get mask
