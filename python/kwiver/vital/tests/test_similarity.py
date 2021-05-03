"""
ckwg +31
Copyright 2016-2020 by Kitware, Inc.
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

tests for Similarity class

"""
from __future__ import print_function
import unittest, pytest


import numpy as np

from kwiver.vital.types import (
    RotationF,
    RotationD,
    SimilarityF,
    SimilarityD
)


class TestSimiliarity (unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.s = 2.4
        cls.r = RotationD([0.1, -1.5, 2.0])
        cls.r_f = RotationF([0.1, -1.5, 2.0])
        cls.t = [1, -2, 5]

    def check_members_equal(self, sim, exp_type, exp_scale, exp_rot_mat, exp_trans, prec):
        self.assertEqual(sim.type_name, exp_type)
        np.testing.assert_almost_equal(sim.scale, exp_scale, prec)
        np.testing.assert_array_almost_equal(sim.rotation.matrix(), exp_rot_mat, prec)
        np.testing.assert_array_almost_equal(sim.translation, exp_trans, prec)

    def test_new_default(self):
        self.check_members_equal(SimilarityD(), 'd', 1, RotationD().matrix(), [0, 0, 0], 15)
        self.check_members_equal(SimilarityF(), 'f', 1, RotationF().matrix(), [0, 0, 0], 6)


    def test_new_from_rot(self):
        sim = SimilarityD(self.s, self.r, self.t)
        self.check_members_equal(sim, 'd', self.s, self.r.matrix(), self.t, 15)

        sim = SimilarityF(self.s, self.r_f, self.t)
        self.check_members_equal(sim, 'f', self.s, self.r_f.matrix(), self.t, 6)

    def test_new_from_mat(self):
        sim = SimilarityD(self.s, self.r, self.t)
        sim_cpy = SimilarityD(sim.matrix())
        self.check_members_equal(sim_cpy, 'd', sim.scale, sim.rotation.matrix(), sim.translation, 15)

        sim = SimilarityF(self.s, self.r_f, self.t)
        sim_cpy = SimilarityF(sim.matrix())
        self.check_members_equal(sim_cpy, 'f', sim.scale, sim.rotation.matrix(), sim.translation, 6)

    def test_copy_constructor(self):
        sim = SimilarityD(self.s, self.r, self.t)
        sim_cpy = SimilarityD(sim)
        self.check_members_equal(sim_cpy, 'd', sim.scale, sim.rotation.matrix(), sim.translation, 15)

        sim = SimilarityF(self.s, self.r_f, self.t)
        sim_cpy = SimilarityF(sim)
        self.check_members_equal(sim_cpy, 'f', sim.scale, sim.rotation.matrix(), sim.translation, 6)

    def test_copy_constructor_different_types(self):
        sim = SimilarityF(self.s, self.r_f, self.t)
        sim_cpy = SimilarityD(sim)
        self.check_members_equal(sim_cpy, 'd', sim.scale, sim.rotation.matrix(), sim.translation, 6)

        sim = SimilarityD(self.s, self.r, self.t)
        sim_cpy = SimilarityF(sim)
        self.check_members_equal(sim_cpy, 'f', sim.scale, sim.rotation.matrix(), sim.translation, 6)

    def test_equals(self):
        # Double
        s1 = SimilarityD()
        s2 = SimilarityD()
        assert(s1 == s2)

        s3 = SimilarityD(self.s, self.r, self.t)
        s4 = SimilarityD(self.s, self.r, self.t)
        assert(s3 == s4)
        self.assertFalse(s1 == s3)

        # Float
        s1 = SimilarityF()
        s2 = SimilarityF()
        assert(s1 == s2)

        s3 = SimilarityF(self.s, self.r_f, self.t)
        s4 = SimilarityF(self.s, self.r_f, self.t)
        assert(s3 == s4)
        self.assertFalse(s1 == s3)

    def test_not_equal(self):
        # Double
        s1 = SimilarityD()
        s2 = SimilarityD()
        self.assertFalse(s1 != s2)

        s3 = SimilarityD(self.s, self.r, self.t)
        s4 = SimilarityD(self.s, self.r, self.t)
        self.assertFalse(s3 != s4)
        assert(s1 != s3)

        # Float
        s1 = SimilarityF()
        s2 = SimilarityF()
        self.assertFalse(s1 != s2)

        s3 = SimilarityF(self.s, self.r_f, self.t)
        s4 = SimilarityF(self.s, self.r_f, self.t)
        self.assertFalse(s3 != s4)
        assert(s1 != s3)

    def test_get_scale(self):
        s = SimilarityD()
        self.assertEqual(s.scale, 1.0)

        s = SimilarityD(self.s, self.r, self.t)
        self.assertEqual(s.scale, self.s)

    def test_get_rotation(self):
        s = SimilarityD()
        np.testing.assert_array_almost_equal(s.rotation.matrix(), RotationD().matrix())

        s = SimilarityD(self.s, self.r, self.t)
        np.testing.assert_array_almost_equal(s.rotation.matrix(), self.r.matrix())

    def test_get_translation(self):
        s = SimilarityD()
        np.testing.assert_array_equal(s.translation, [0,0,0])

        s = SimilarityD(self.s, self.r, self.t)
        np.testing.assert_array_equal(s.translation, self.t)

    def test_convert_matrix(self):
        sim = SimilarityD()
        np.testing.assert_array_equal(sim.matrix(), np.eye(4))

        sim1 = SimilarityD(self.s, self.r, self.t)
        mat1 = sim1.matrix()
        sim2 = SimilarityD(mat1)
        mat2 = sim2.matrix()

        print("Sim1:", sim1.matrix())
        print("Sim2:", sim2.matrix())

        np.testing.assert_array_almost_equal(mat1, mat2, decimal=14)

    def test_mul_sim(self):
        s1 = SimilarityD(self.s, self.r, self.t)
        s2 = SimilarityD(0.75,
                        RotationD([-0.5, -0.5, 1.0]),
                        [4, 6.5, 8])

        sim_comp = (s1 * s2).matrix()
        mat_comp = np.dot(s1.matrix(), s2.matrix())
        print('sim12 comp:\n', sim_comp)
        print('mat comp:\n', mat_comp)
        print('sim - mat:\n', sim_comp - mat_comp)
        self.assertAlmostEqual(
            np.linalg.norm(sim_comp - mat_comp, 2),
            0., 12
        )

    def test_mul_vector(self):
        s = SimilarityD(self.s, self.r, self.t)

        v1 = [4, 2.1, 9.125]
        v2 = s * v1
        v3 = s.inverse() * v2
        self.assertFalse(np.allclose(v1, v2))
        self.assertTrue(np.allclose(v1, v3))

    def test_mul_fail(self):
        s = SimilarityD(self.s, self.r, self.t)
        with pytest.raises(TypeError):
            s * 0
        with pytest.raises(TypeError):
            s * 'foo'

    def test_inverse(self):
        # Inverse of identity is itself
        s = SimilarityD()
        self.assertEqual(s, s.inverse())

        s = SimilarityD(self.s, self.r, self.t)
        s_i = s.inverse()
        i = s * s_i
        # Similarity composed with inverse should be identity
        self.assertAlmostEqual(i.scale, 1., 14)
        self.assertAlmostEqual(i.rotation.angle(), .0, 14)
        self.assertAlmostEqual(np.linalg.norm(i.translation, 2),
                                       0., 12)
