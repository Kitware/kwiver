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

Test Python interface to vital::bounding_box

"""
# -*- coding: utf-8 -*-
import nose.tools

from vital.types import BoundingBox


class TestVitalBoundingBox (object):

    def test_new(self):
        ul = [10.0, 10.0]
        ur = [25.5, 35.0]
        h = 15.3
        w = 25.4
        bbox = BoundingBox(ul, ur)
        bbox = BoundingBox(ul, w, h)
        bbox = BoundingBox(10, 12, 45, 56)

    def test_center(self):
        bbox = BoundingBox(10, 12, 45, 56)
        ctr = bbox.center()
        nose.tools.assert_equal(ctr[0], (10+45)/2)
        nose.tools.assert_equal(ctr[1], (12+56)/2)

    def test_upper_left(self):
        bbox = BoundingBox(10, 12, 45, 56)
        ul = bbox.upper_left()
        nose.tools.assert_equal(ul[0], 10)
        nose.tools.assert_equal(ul[1], 12)

    def test_lower_left(self):
        bbox = BoundingBox(10, 12, 45, 56)
        lr = bbox.lower_right()
        nose.tools.assert_equal(lr[0], 45)
        nose.tools.assert_equal(lr[1], 56)

    def test_min_x(self):
        bbox = BoundingBox(10, 12, 45, 56)
        nose.tools.assert_equal(bbox.min_x(), 10)

    def test_max_x(self):
        bbox = BoundingBox(10, 12, 45, 56)
        nose.tools.assert_equal(bbox.max_x(), 45)

    def test_min_y(self):
        bbox = BoundingBox(10, 12, 45, 56)
        nose.tools.assert_equal(bbox.min_y(), 12)

    def test_max_y(self):
        bbox = BoundingBox(10, 12, 45, 56)
        nose.tools.assert_equal(bbox.max_y(), 56)

    def test_width(self):
        bbox = BoundingBox(10, 12, 45, 56)
        nose.tools.assert_equal(bbox.width(), 45-10)

    def test_height(self):
        bbox = BoundingBox(10, 12, 45, 56)
        nose.tools.assert_equal(bbox.height(), 56-12)

    def test_area(self):
        bbox = BoundingBox(10, 12, 45, 56)
        nose.tools.assert_equal(bbox.height(), (45-10)*(56-12))
