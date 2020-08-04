import unittest
import nose.tools
import numpy as np


from kwiver.vital.types import DetectedObjectType as DOT


class TestDetectedObject(unittest.TestCase):

    def test_constructor(self):
        DOT()
        DOT(np.array(["name1","class_name2","class3"]),np.array([1.0,2.3,3.14]))
        DOT("name", 2.0)

    def test_methods(self):
        t = DOT(np.array(["name1","class_name2","class3"]),np.array([1.0,2.3,3.14]))
        
        # str/repr/itr
        self.assertIsInstance(str(t),str)
        self.assertIsInstance(t.__repr__(), str)
        itr = iter(t)
        self.assertIsInstance(next(itr),tuple)


        # Has Class Name
        self.assertTrue(t.has_class_name("name1"))
        self.assertFalse(t.has_class_name("foo"))

        # Score
        self.assertEqual(t.score("class_name2"),2.3)

        # Get most likely
        self.assertEqual(t.get_most_likely_class(),"class3")        
        self.assertEqual(3.14,t.get_most_likely_score())

        # Set Score
        t.set_score("foo1",3.8)
        self.assertEqual(3.8,t.score("foo1"))
        t.set_score("foo2",3.9)
        self.assertTrue(t.has_class_name("foo2"))
        self.assertEqual(t.score("foo2"),3.9)

        # Delete Score
        t.delete_score("foo1")
        self.assertFalse(t.has_class_name("foo1"))

        # Class Names
        np.testing.assert_array_equal(t.class_names(), np.array(["foo2","class3","class_name2","name1"]))
        np.testing.assert_array_equal(t.class_names(3.0), np.array(["foo2","class3"]))

        np.testing.assert_array_equal(t.all_class_names(),np.array(["class3","class_name2","foo1","foo2","name","name1"]))



