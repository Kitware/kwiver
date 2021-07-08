#!/usr/bin/env python
#ckwg +28
# Copyright 2011-2020 by Kitware, Inc.
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

from kwiver.sprokit.util.test import expect_exception, find_tests, run_test, test_error

def test_import():
    try:
        import kwiver.sprokit.pipeline.datum
    except:
        test_error("Failed to import the datum module")


def test_new():
    from kwiver.sprokit.pipeline import datum

    d = datum.new('test_datum')

    if not d.type() == datum.DatumType.data:
        test_error("Datum type mismatch")

    if len(d.get_error()):
        test_error("A data datum has an error string")

    p = d.get_datum()

    if p is None:
        test_error("A data datum has None as its data")


def test_empty():
    from  kwiver.sprokit.pipeline import datum

    d = datum.empty()

    if not d.type() == datum.DatumType.empty:
        test_error("Datum type mismatch")

    if len(d.get_error()):
        test_error("An empty datum has an error string")

    p = d.get_datum()

    if p is not None:
        test_error("An empty datum does not have None as its data")


def test_flush():
    from  kwiver.sprokit.pipeline import datum

    d = datum.flush()

    if not d.type() == datum.DatumType.flush:
        test_error("Datum type mismatch")

    if len(d.get_error()):
        test_error("A flush datum has an error string")

    p = d.get_datum()

    if p is not None:
        test_error("A flush datum does not have None as its data")


def test_complete():
    from  kwiver.sprokit.pipeline import datum

    d = datum.complete()

    if not d.type() == datum.DatumType.complete:
        test_error("Datum type mismatch")

    if len(d.get_error()):
        test_error("A complete datum has an error string")

    p = d.get_datum()

    if p is not None:
        test_error("A complete datum does not have None as its data")


def test_error_():
    from  kwiver.sprokit.pipeline import datum

    err = 'An error'

    d = datum.error(err)

    if not d.type() == datum.DatumType.error:
        test_error("Datum type mismatch")

    if not d.get_error() == err:
        test_error("An error datum did not keep the message")

    p = d.get_datum()

    if p is not None:
        test_error("An error datum does not have None as its data")

def err_ne(expected, actual):
    if expected != actual:
        test_error("Expected value of {}, got {}".format(expected, actual))


def err_is_not(expected, actual):
    if expected is not actual:
        msg = "Expected {} and {} to point to same object"
        test_error(msg.format(expected, actual))

def err_str_ne(expected, actual):
    err_ne(str(expected), str(actual))

# Check the automatic type conversion done by new() and get_datum()
def check_automatic_conversion(val, compare_fxn):
    from kwiver.sprokit.pipeline import datum

    datum_inst = datum.new(val)
    retrieved_val = datum_inst.get_datum()
    compare_fxn(retrieved_val, val)

# Next some basic types
def test_add_get_basic_types():
    from kwiver.sprokit.pipeline import datum

    # Try the typed constructor/get fxns first
    datum_inst = datum.new_int(10)
    retrieved_val = datum_inst.get_int()
    err_ne(retrieved_val, 10)

    datum_inst = datum.new_float(0.5)
    retrieved_val = datum_inst.get_float()
    err_ne(retrieved_val, 0.5)

    datum_inst = datum.new_double(3.14)
    retrieved_val = datum_inst.get_double()
    err_ne(retrieved_val, 3.14)

    datum_inst = datum.new_bool(True)
    retrieved_val = datum_inst.get_bool()
    err_ne(retrieved_val, True)

    datum_inst = datum.new_string("str1")
    retrieved_val = datum_inst.get_string()
    err_ne(retrieved_val, "str1")

    # Now the ones with automatic conversion
    check_automatic_conversion(10, err_ne)
    check_automatic_conversion(0.5, err_ne)
    check_automatic_conversion(True, err_ne)
    check_automatic_conversion("str1", err_ne)

# Next some kwiver vital types that are handled with pointers
def test_add_get_vital_types_by_ptr():
    from kwiver.sprokit.pipeline import datum
    from kwiver.vital import types as kvt

    val = kvt.ImageContainer(kvt.Image())
    datum_inst = datum.new_image_container(val)
    retrieved_val = datum_inst.get_image_container()
    err_is_not(retrieved_val, val)

    val = kvt.DescriptorSet()
    datum_inst = datum.new_descriptor_set(val)
    retrieved_val = datum_inst.get_descriptor_set()
    err_is_not(retrieved_val, val)

    val = kvt.DetectedObjectSet()
    datum_inst = datum.new_detected_object_set(val)
    retrieved_val = datum_inst.get_detected_object_set()
    err_is_not(retrieved_val, val)

    val = kvt.TrackSet()
    datum_inst = datum.new_track_set(val)
    retrieved_val = datum_inst.get_track_set()
    err_is_not(retrieved_val, val)

    val = kvt.ObjectTrackSet()
    datum_inst = datum.new_object_track_set(val)
    retrieved_val = datum_inst.get_object_track_set()
    err_is_not(retrieved_val, val)

    check_automatic_conversion(kvt.ImageContainer(kvt.Image()), err_is_not)
    check_automatic_conversion(kvt.DescriptorSet(), err_is_not)
    check_automatic_conversion(kvt.DetectedObjectSet(), err_is_not)
    check_automatic_conversion(kvt.TrackSet(), err_is_not)
    check_automatic_conversion(kvt.ObjectTrackSet(), err_is_not)

# Next some bound native C++ types
def test_add_get_cpp_types():
    from kwiver.sprokit.pipeline import datum

    datum_inst = datum.new_double_vector(datum.VectorDouble([3.14, 4.14]))
    retrieved_val = datum_inst.get_double_vector()
    err_ne(retrieved_val, datum.VectorDouble([3.14, 4.14]))

    datum_inst = datum.new_string_vector(datum.VectorString(["s00", "s01"]))
    retrieved_val = datum_inst.get_string_vector()
    err_ne(retrieved_val, datum.VectorString(["s00", "s01"]))

    datum_inst = datum.new_uchar_vector(datum.VectorUChar([100, 101]))
    retrieved_val = datum_inst.get_uchar_vector()
    err_ne(retrieved_val, datum.VectorUChar([100, 101]))

    check_automatic_conversion(datum.VectorDouble([3.14, 4.14]), err_ne)
    check_automatic_conversion(datum.VectorString(["s00", "s01"]), err_ne)
    check_automatic_conversion(datum.VectorUChar([100, 101]), err_ne)

# Next kwiver vital types
def test_add_get_vital_types():
    from kwiver.sprokit.pipeline import datum
    from kwiver.vital import types as kvt

    datum_inst = datum.new_bounding_box(kvt.BoundingBoxD(1, 1, 2, 2))
    retrieved_val = datum_inst.get_bounding_box()
    err_ne(retrieved_val, kvt.BoundingBoxD(1, 1, 2, 2))

    datum_inst = datum.new_timestamp(kvt.Timestamp(123, 1))
    retrieved_val = datum_inst.get_timestamp()
    err_ne(retrieved_val, kvt.Timestamp(123, 1))

    datum_inst = datum.new_f2f_homography(kvt.F2FHomography(1))
    retrieved_val = datum_inst.get_f2f_homography()
    err_str_ne(retrieved_val, kvt.F2FHomography(1))

    check_automatic_conversion(kvt.BoundingBoxD(1, 1, 2, 2), err_ne)
    check_automatic_conversion(kvt.Timestamp(123, 1), err_ne)
    check_automatic_conversion(kvt.F2FHomography(1), err_str_ne)

# Want to make sure data inside a datum created with the automatic
# conversion constructor can be retrieved with a type specific getter, and
# vice versa
def test_mix_new_and_get():
    from kwiver.sprokit.pipeline import datum
    from kwiver.vital import types as kvt

    # Try creating with generic constructor first, retrieving with
    # type specific get function
    datum_inst = datum.new("string_value")
    err_ne(datum_inst.get_string(), "string_value")

    datum_inst = datum.new(kvt.Timestamp(1000000000, 10))
    err_ne(datum_inst.get_timestamp(), kvt.Timestamp(1000000000, 10))

    datum_inst = datum.new(datum.VectorString(["element1", "element2"]))
    err_ne(datum_inst.get_string_vector(), datum.VectorString(["element1", "element2"]))

    # Now try the opposite
    datum_inst = datum.new_string("string_value")
    err_ne(datum_inst.get_datum(), "string_value")

    datum_inst = datum.new_timestamp(kvt.Timestamp(1000000000, 10))
    err_ne(datum_inst.get_datum(), kvt.Timestamp(1000000000, 10))

    datum_inst = datum.new_string_vector(datum.VectorString(["element1", "element2"]))
    err_ne(datum_inst.get_datum(), datum.VectorString(["element1", "element2"]))

# Make sure that None isn't acceptable, even for pointers
def test_new_with_none():
    from kwiver.sprokit.pipeline import datum
    from kwiver.vital import types as kvt

    expect_exception(
        "attempting to store None as a string vector",
        TypeError,
        datum.new_string_vector,
        None,
    )

    expect_exception(
        "attempting to store None as a track_set",
        TypeError,
        datum.new_track_set,
        None,
    )

    expect_exception(
        "attempting to store None as a timestamp",
        TypeError,
        datum.new_timestamp,
        None,
    )

    # Should also fail for the automatic type conversion
    expect_exception(
        "attempting to store none through automatic conversion",
        TypeError,
        datum.new,
        None,
    )

if __name__ == '__main__':
    import sys

    if len(sys.argv) != 2:
        test_error("Expected two arguments")
        sys.exit(1)

    testname = sys.argv[1]


    run_test(testname, find_tests(locals()))
