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
        import kwiver.sprokit.adapters.adapter_data_set
    except:
        test_error("Failed to import the adapter_data_set module")


def check_port_val_equals(expected_val, actual_val, portname):
    if expected_val != actual_val:
        test_error("Value mismatch on port {}".format(portname))


def test_create():
    from kwiver.sprokit.adapters import adapter_data_set

    adapter_data_set.create()
    adapter_data_set.create(adapter_data_set.DataSetType.data)
    adapter_data_set.create(adapter_data_set.DataSetType.end_of_input)


def check_type(ads_def, ads_data, ads_eoi):
    from kwiver.sprokit.adapters import adapter_data_set

    if ads_def.type() != adapter_data_set.DataSetType.data:
        test_error("adapter_data_set type mismatch: constructor with default arg")

    if ads_data.type() != adapter_data_set.DataSetType.data:
        test_error("adapter_data_set type mismatch: constructor with data arg")

    if ads_eoi.type() != adapter_data_set.DataSetType.end_of_input:
        test_error("adapter_data_set type mismatch: constructor with end_of_input arg")



def test_enums():
    from kwiver.sprokit.adapters import adapter_data_set

    if int(adapter_data_set.DataSetType.data) != 1:
        test_error("adapter_data_set enum value mismatch: data")

    if int(adapter_data_set.DataSetType.end_of_input) != 2:
        test_error("adapter_data_set enum value mismatch: end_of_input")


def test_is_end_of_data():
    from kwiver.sprokit.adapters import adapter_data_set

    ads_def = adapter_data_set.create()  # test default argument
    ads_data = adapter_data_set.create(adapter_data_set.DataSetType.data)
    ads_eoi = adapter_data_set.create(adapter_data_set.DataSetType.end_of_input)

    if ads_def.is_end_of_data():
        test_error(
            'adapter data set of type "data" is empty: constructor with default arg'
        )

    if ads_data.is_end_of_data():
        test_error(
            'adapter data set of type "data" is empty: constructor with data arg'
        )

    if not ads_eoi.is_end_of_data():
        test_error('adapter_data_set of type "end_of_input" is not empty')

# adds and retrieves val to/from the
# adapter_data_set instance twice
# once with the add/get fxn specified,
# once with the add/get function for py_objects

def add_get_helper(
    instance,
    instance_add_fxn,
    instance_get_fxn,
    val,
    data_type_str,
):

    # First the specified add/get fxns
    portname = data_type_str + "_port"
    instance_add_fxn(portname, val)
    instance_get_fxn(portname)  # This throws a runtime error if the port is not found

    # Next the python handled add/get fxns
    py_portname = "py_" + portname
    instance.add_value(py_portname, val)
    instance.get_port_data(py_portname)

def overwrite_helper(
    instance_add_fxn,
    instance_get_fxn,
    val,
    new_data_type_str,
    portname
):
    from kwiver.sprokit.pipeline import datum
    instance_add_fxn(portname, val)
    try:
        retrieved_val = instance_get_fxn(portname)
    except RuntimeError:
        test_error("Failed to get object of type {} after attempting overwrite".format(new_data_type_str))
    else:
        if isinstance(val, datum.Datum):
            val = val.get_datum()
        if retrieved_val != val:
            test_error("Retrieved incorrect value after overwriting with {}".format(new_data_type_str))



def test_api_calls():
    from kwiver.sprokit.adapters import adapter_data_set
    import kwiver.vital.types as kvt
    from kwiver.sprokit.pipeline import datum

    ads = adapter_data_set.create()  # Check constructor with default argument
    ads_data = adapter_data_set.create(adapter_data_set.DataSetType.data)
    ads_eoi = adapter_data_set.create(adapter_data_set.DataSetType.end_of_input)

    check_type(ads, ads_data, ads_eoi)

    # Now do some checks for ads_def
    if not ads.empty():
        test_error("fresh data adapter_data_set instance is not empty")

    # First check a python datum object
    add_get_helper(ads, ads.add_datum, ads.get_port_data, datum.new("d1"), "datum")
    # Next some basic types
    add_get_helper(ads, ads.add_int, ads.get_port_data_int, 10, "int")
    add_get_helper(ads, ads.add_float, ads.get_port_data_float, 0.5, "float")
    add_get_helper(ads, ads.add_string, ads.get_port_data_string, "str1", "string")
    # Next some kwiver vital types that are handled with pointers
    add_get_helper(ads, ads.add_image_container, ads.get_port_data_image_container, kvt.ImageContainer(kvt.Image()), "image_container")
    add_get_helper(ads, ads.add_descriptor_set, ads.get_port_data_descriptor_set, kvt.DescriptorSet(), "descriptor_set")
    add_get_helper(ads, ads.add_detected_object_set, ads.get_port_data_detected_object_set, kvt.DetectedObjectSet(), "detected_object_set")
    add_get_helper(ads, ads.add_track_set, ads.get_port_data_track_set, kvt.TrackSet(), "track_set")
    add_get_helper(ads, ads.add_object_track_set, ads.get_port_data_object_track_set, kvt.ObjectTrackSet(), "object_track_set")
    # Next some bound native C++ types
    add_get_helper(ads, ads.add_double_vector, ads.get_port_data_double_vector, datum.VectorDouble([3.14, 4.14]), "double_vector")
    add_get_helper(ads, ads.add_string_vector, ads.get_port_data_string_vector, datum.VectorString(["s00", "s01"]), "string_vector")
    add_get_helper(ads, ads.add_uchar_vector, ads.get_port_data_uchar_vector, datum.VectorUChar([100, 101]), "uchar_vector")
    # Now try creating datums of these bound types
    add_get_helper(ads, ads.add_datum, ads.get_port_data_double_vector, datum.new_double_vector(datum.VectorDouble([6.3, 8.9])), "datum_double_vector")
    add_get_helper(ads, ads.add_datum, ads.get_port_data_string_vector, datum.new_string_vector(datum.VectorString(["foo", "bar"])), "datum_string_vector")
    add_get_helper(ads, ads.add_datum, ads.get_port_data_uchar_vector, datum.new_uchar_vector(datum.VectorUChar([102, 103])), "datum_uchar_vector")
    # Next kwiver vital types
    add_get_helper(ads, ads.add_bounding_box, ads.get_port_data_bounding_box, kvt.BoundingBox(1, 1, 2, 2), "bounding_box")
    add_get_helper(ads, ads.add_timestamp, ads.get_port_data_timestamp, kvt.Timestamp(), "timestamp")
    add_get_helper(ads, ads.add_f2f_homography, ads.get_port_data_f2f_homography, kvt.F2FHomography(1), "f2f_homography")


    # Now test overwriting
    # First check overwriting port with the same datum type
    OVERWRITE_PORT = "test_overwrite_port"

    ads.add_datum(OVERWRITE_PORT, datum.new("d2"))
    overwrite_helper(ads.add_datum, ads.get_port_data, datum.new("d3"), "datum_string", OVERWRITE_PORT)
    overwrite_helper(ads.add_datum, ads.get_port_data, datum.new(12), "datum_int", OVERWRITE_PORT)
    overwrite_helper(ads.add_string_vector, ads.get_port_data_string_vector, datum.VectorString(["baz", "qux"]), "string_vector", OVERWRITE_PORT)
    overwrite_helper(ads.add_value, ads.get_port_data, 15, "int", OVERWRITE_PORT)
    overwrite_helper(ads.add_double_vector, ads.get_port_data_double_vector, datum.VectorDouble([4, 8]), "double_vector", OVERWRITE_PORT)

    # Now test iter()
    for el in ads:
        pass



if __name__ == "__main__":
    import os
    import sys

    if not len(sys.argv) == 4:
        test_error("Expected three arguments")
        sys.exit(1)

    testname = sys.argv[1]

    os.chdir(sys.argv[2])

    sys.path.append(sys.argv[3])

    from kwiver.sprokit.util.test import *

    run_test(testname, find_tests(locals()))
