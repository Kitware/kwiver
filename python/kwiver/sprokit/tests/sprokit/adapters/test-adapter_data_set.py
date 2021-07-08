#!/usr/bin/env python
# ckwg +28
# Copyright 2020 by Kitware, Inc.
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

from kwiver.sprokit.util.test import (
    expect_exception,
    find_tests,
    run_test,
    test_error,
)

from kwiver.sprokit.pipeline import datum


def test_import():
    try:
        import kwiver.sprokit.adapters.adapter_data_set
    except:
        test_error("Failed to import the adapter_data_set module")


def test_create():
    from kwiver.sprokit.adapters import adapter_data_set

    adapter_data_set.AdapterDataSet.create()
    adapter_data_set.AdapterDataSet.create(adapter_data_set.DataSetType.data)
    adapter_data_set.AdapterDataSet.create(
        adapter_data_set.DataSetType.end_of_input
    )


def check_type():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = (
        adapter_data_set.AdapterDataSet.create()
    )  # Check constructor with default argument
    ads_data = adapter_data_set.AdapterDataSet.create(
        adapter_data_set.DataSetType.data
    )
    ads_eoi = adapter_data_set.AdapterDataSet.create(
        adapter_data_set.DataSetType.end_of_input
    )

    if ads_def.type() != adapter_data_set.DataSetType.data:
        test_error(
            "adapter_data_set type mismatch: constructor with default arg"
        )

    if ads_data.type() != adapter_data_set.DataSetType.data:
        test_error("adapter_data_set type mismatch: constructor with data arg")

    if ads_eoi.type() != adapter_data_set.DataSetType.end_of_input:
        test_error(
            "adapter_data_set type mismatch: constructor with end_of_input arg"
        )


def test_enums():
    from kwiver.sprokit.adapters import adapter_data_set

    if int(adapter_data_set.DataSetType.data) != 1:
        test_error("adapter_data_set enum value mismatch: data")

    if int(adapter_data_set.DataSetType.end_of_input) != 2:
        test_error("adapter_data_set enum value mismatch: end_of_input")


def test_is_end_of_data():
    from kwiver.sprokit.adapters import adapter_data_set

    ads_def = adapter_data_set.AdapterDataSet.create()  # test default argument
    ads_data = adapter_data_set.AdapterDataSet.create(
        adapter_data_set.DataSetType.data
    )
    ads_eoi = adapter_data_set.AdapterDataSet.create(
        adapter_data_set.DataSetType.end_of_input
    )

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


def overwrite_helper(
    instance_add_fxn, instance_get_fxn, val, new_data_type_str, portname
):
    from kwiver.sprokit.adapters import adapter_data_set

    instance_add_fxn(portname, val)
    try:
        retrieved_val = instance_get_fxn(portname)
    except RuntimeError:
        test_error(
            "Failed to get object of type {} after attempting overwrite".format(
                new_data_type_str
            )
        )
    else:
        if isinstance(val, datum.Datum):
            val = val.get_datum()
        if retrieved_val != val:
            test_error(
                "Retrieved incorrect value after overwriting with {}".format(
                    new_data_type_str
                )
            )


def test_empty():
    from kwiver.sprokit.adapters import adapter_data_set

    if not adapter_data_set.AdapterDataSet.create().empty():
        test_error("fresh data adapter_data_set instance is not empty")


# First check a python datum object
def test_add_get_datum():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()

    val = "d1"
    d = datum.new(val)
    ads.add_datum("datum_port", d)
    retrieved_val = ads.get_port_data("datum_port")
    err_ne(val, retrieved_val)


def err_ne(expected, actual):
    if expected != actual:
        test_error("Expected value of {}, got {}".format(expected, actual))


def err_is_not(expected, actual):
    if expected is not actual:
        msg = "Expected {} and {} to point to same object"
        test_error(msg.format(expected, actual))


def err_is_not_none(actual):
    if actual is not None:
        msg = "Expected to get None, got {} instead"
        test_error(msg.format(actual))


# Next some basic types
def test_add_get_basic_types():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()

    val = 10
    # Typed methods
    ads._add_int("int_port", val)
    retrieved_val1 = ads._get_port_data_int("int_port")
    retrieved_val2 = ads._value_int("int_port")
    retrieved_val3 = ads._value_or_int("int_port")

    # Non typed methods
    ads.add_value("int_port2", val)
    ads["int_port3"] = val
    retrieved_val4 = ads.get_port_data("int_port2")
    retrieved_val5 = ads.value("int_port3")
    retrieved_val6 = ads.value_or("int_port3")
    retrieved_val7 = ads["int_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = 0.5
    # Typed methods
    ads._add_float("float_port", val)
    retrieved_val1 = ads._get_port_data_float("float_port")
    retrieved_val2 = ads._value_float("float_port")
    retrieved_val3 = ads._value_or_float("float_port")

    # Non typed methods
    ads.add_value("float_port2", val)
    ads["float_port3"] = val
    retrieved_val4 = ads.get_port_data("float_port2")
    retrieved_val5 = ads.value("float_port3")
    retrieved_val6 = ads.value_or("float_port3")
    retrieved_val7 = ads["float_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = 3.14
    # Typed methods
    ads._add_double("double_port", val)
    retrieved_val1 = ads._get_port_data_double("double_port")
    retrieved_val2 = ads._value_double("double_port")
    retrieved_val3 = ads._value_or_double("double_port")

    # Non typed methods
    ads.add_value("double_port2", val)
    ads["double_port3"] = val
    retrieved_val4 = ads.get_port_data("double_port2")
    retrieved_val5 = ads.value("double_port3")
    retrieved_val6 = ads.value_or("double_port3")
    retrieved_val7 = ads["double_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = True
    # Typed methods
    ads._add_bool("bool_port", val)
    retrieved_val1 = ads._get_port_data_bool("bool_port")
    retrieved_val2 = ads._value_bool("bool_port")
    retrieved_val3 = ads._value_or_bool("bool_port")

    # Non typed methods
    ads.add_value("bool_port2", val)
    ads["bool_port3"] = val
    retrieved_val4 = ads.get_port_data("bool_port2")
    retrieved_val5 = ads.value("bool_port3")
    retrieved_val6 = ads.value_or("bool_port3")
    retrieved_val7 = ads["bool_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = "str1"
    # Typed methods
    ads._add_string("string_port", val)
    retrieved_val1 = ads._get_port_data_string("string_port")
    retrieved_val2 = ads._value_string("string_port")
    retrieved_val3 = ads._value_or_string("string_port")

    # Non typed methods
    ads.add_value("string_port2", val)
    ads["string_port3"] = val
    retrieved_val4 = ads.get_port_data("string_port2")
    retrieved_val5 = ads.value("string_port3")
    retrieved_val6 = ads.value_or("string_port3")
    retrieved_val7 = ads["string_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)


# Make sure that value_or works as expected when data is not found
def test_value_or_basic_types():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()

    val = 10
    # Typed
    retrieved_val1 = ads._value_or_int("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    # Check default values
    err_is_not_none(ads.value_or("nonexistant_port"))
    err_is_not_none(ads._value_or_int("nonexistant_port"))

    val = 0.5
    # Typed
    retrieved_val1 = ads._value_or_float("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_is_not_none(ads._value_or_float("nonexistant_port"))

    val = 3.14
    # Typed
    retrieved_val1 = ads._value_or_double("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_is_not_none(ads._value_or_double("nonexistant_port"))

    val = True
    # Typed
    retrieved_val1 = ads._value_or_bool("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_is_not_none(ads._value_or_bool("nonexistant_port"))

    val = "str1"
    # Typed
    retrieved_val1 = ads._value_or_string("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_is_not_none(ads._value_or_string("nonexistant_port"))


# Next some kwiver vital types that are handled with pointers
def test_add_get_vital_types_by_ptr():
    from kwiver.sprokit.adapters import adapter_data_set
    from kwiver.vital import types as kvt

    ads = adapter_data_set.AdapterDataSet.create()

    val = kvt.ImageContainer(kvt.Image())
    # Typed methods
    ads._add_image_container("image_container_port", val)
    retrieved_val1 = ads._get_port_data_image_container("image_container_port")
    retrieved_val2 = ads._value_image_container("image_container_port")
    retrieved_val3 = ads._value_or_image_container("image_container_port")

    # Non typed methods
    ads.add_value("image_container_port2", val)
    ads["image_container_port3"] = val
    retrieved_val4 = ads.get_port_data("image_container_port2")
    retrieved_val5 = ads.value("image_container_port3")
    retrieved_val6 = ads.value_or("image_container_port3")
    retrieved_val7 = ads["image_container_port3"]
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not(val, retrieved_val3)
    err_is_not(val, retrieved_val4)
    err_is_not(val, retrieved_val5)
    err_is_not(val, retrieved_val6)
    err_is_not(val, retrieved_val7)

    val = kvt.DescriptorSet()
    # Typed methods
    ads._add_descriptor_set("descriptor_set_port", val)
    retrieved_val1 = ads._get_port_data_descriptor_set("descriptor_set_port")
    retrieved_val2 = ads._value_descriptor_set("descriptor_set_port")
    retrieved_val3 = ads._value_or_descriptor_set("descriptor_set_port")

    # Non typed methods
    ads.add_value("descriptor_set_port2", val)
    ads["descriptor_set_port3"] = val
    retrieved_val4 = ads.get_port_data("descriptor_set_port2")
    retrieved_val5 = ads.value("descriptor_set_port3")
    retrieved_val6 = ads.value_or("descriptor_set_port3")
    retrieved_val7 = ads["descriptor_set_port3"]
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not(val, retrieved_val3)
    err_is_not(val, retrieved_val4)
    err_is_not(val, retrieved_val5)
    err_is_not(val, retrieved_val6)
    err_is_not(val, retrieved_val7)

    val = kvt.DetectedObjectSet()
    # Typed methods
    ads._add_detected_object_set("detected_object_set_port", val)
    retrieved_val1 = ads._get_port_data_detected_object_set(
        "detected_object_set_port"
    )
    retrieved_val2 = ads._value_detected_object_set("detected_object_set_port")
    retrieved_val3 = ads._value_or_detected_object_set(
        "detected_object_set_port"
    )

    # Non typed methods
    ads.add_value("detected_object_set_port2", val)
    ads["detected_object_set_port3"] = val
    retrieved_val4 = ads.get_port_data("detected_object_set_port2")
    retrieved_val5 = ads.value("detected_object_set_port3")
    retrieved_val6 = ads.value_or("detected_object_set_port3")
    retrieved_val7 = ads["detected_object_set_port3"]
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not(val, retrieved_val3)
    err_is_not(val, retrieved_val4)
    err_is_not(val, retrieved_val5)
    err_is_not(val, retrieved_val6)
    err_is_not(val, retrieved_val7)

    val = kvt.TrackSet()
    # Typed methods
    ads._add_track_set("track_set_port", val)
    retrieved_val1 = ads._get_port_data_track_set("track_set_port")
    retrieved_val2 = ads._value_track_set("track_set_port")
    retrieved_val3 = ads._value_or_track_set("track_set_port")

    # Non typed methods
    ads.add_value("track_set_port2", val)
    ads["track_set_port3"] = val
    retrieved_val4 = ads.get_port_data("track_set_port2")
    retrieved_val5 = ads.value("track_set_port3")
    retrieved_val6 = ads.value_or("track_set_port3")
    retrieved_val7 = ads["track_set_port3"]
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not(val, retrieved_val3)
    err_is_not(val, retrieved_val4)
    err_is_not(val, retrieved_val5)
    err_is_not(val, retrieved_val6)
    err_is_not(val, retrieved_val7)

    val = kvt.ObjectTrackSet()
    # Typed methods
    ads._add_object_track_set("object_track_set_port", val)
    retrieved_val1 = ads._get_port_data_object_track_set(
        "object_track_set_port"
    )
    retrieved_val2 = ads._value_object_track_set("object_track_set_port")
    retrieved_val3 = ads._value_or_object_track_set("object_track_set_port")

    # Non typed methods
    ads.add_value("object_track_set_port2", val)
    ads["object_track_set_port3"] = val
    retrieved_val4 = ads.get_port_data("object_track_set_port2")
    retrieved_val5 = ads.value("object_track_set_port3")
    retrieved_val6 = ads.value_or("object_track_set_port3")
    retrieved_val7 = ads["object_track_set_port3"]
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not(val, retrieved_val3)
    err_is_not(val, retrieved_val4)
    err_is_not(val, retrieved_val5)
    err_is_not(val, retrieved_val6)
    err_is_not(val, retrieved_val7)


def test_value_or_vital_types_by_ptr():
    from kwiver.sprokit.adapters import adapter_data_set
    import kwiver.vital.types as kvt

    ads = adapter_data_set.AdapterDataSet.create()

    val = kvt.ImageContainer(kvt.Image())
    # Typed
    retrieved_val1 = ads._value_or_image_container("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not_none(ads._value_or_image_container("nonexistant_port"))

    val = kvt.DescriptorSet()
    # Typed
    retrieved_val1 = ads._value_or_descriptor_set("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not_none(ads._value_or_descriptor_set("nonexistant_port"))

    val = kvt.DetectedObjectSet()
    # Typed
    retrieved_val1 = ads._value_or_detected_object_set("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not_none(ads._value_or_detected_object_set("nonexistant_port"))

    val = kvt.TrackSet()
    # Typed
    retrieved_val1 = ads._value_or_track_set("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not_none(ads._value_or_track_set("nonexistant_port"))

    val = kvt.ObjectTrackSet()
    # Typed
    retrieved_val1 = ads._value_or_object_track_set("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_is_not(val, retrieved_val1)
    err_is_not(val, retrieved_val2)
    err_is_not_none(ads._value_or_object_track_set("nonexistant_port"))


# Next some bound native C++ types
def test_add_get_cpp_types():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()

    val = adapter_data_set.VectorDouble([3.14, 4.14])
    # Typed methods
    ads._add_double_vector("double_vector_port", val)
    retrieved_val1 = ads._get_port_data_double_vector("double_vector_port")
    retrieved_val2 = ads._value_double_vector("double_vector_port")
    retrieved_val3 = ads._value_or_double_vector("double_vector_port")

    # Non typed methods
    ads.add_value("double_vector_port2", val)
    ads["double_vector_port3"] = val
    retrieved_val4 = ads.get_port_data("double_vector_port2")
    retrieved_val5 = ads.value("double_vector_port3")
    retrieved_val6 = ads.value_or("double_vector_port3")
    retrieved_val7 = ads["double_vector_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = adapter_data_set.VectorString(["s00", "s01"])
    # Typed methods
    ads._add_string_vector("string_vector_port", val)
    retrieved_val1 = ads._get_port_data_string_vector("string_vector_port")
    retrieved_val2 = ads._value_string_vector("string_vector_port")
    retrieved_val3 = ads._value_or_string_vector("string_vector_port")

    # Non typed methods
    ads.add_value("string_vector_port2", val)
    ads["string_vector_port3"] = val
    retrieved_val4 = ads.get_port_data("string_vector_port2")
    retrieved_val5 = ads.value("string_vector_port3")
    retrieved_val6 = ads.value_or("string_vector_port3")
    retrieved_val7 = ads["string_vector_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = adapter_data_set.VectorUChar([100, 101])
    # Typed methods
    ads._add_uchar_vector("uchar_vector_port", val)
    retrieved_val1 = ads._get_port_data_uchar_vector("uchar_vector_port")
    retrieved_val2 = ads._value_uchar_vector("uchar_vector_port")
    retrieved_val3 = ads._value_or_uchar_vector("uchar_vector_port")

    # Non typed methods
    ads.add_value("uchar_vector_port2", val)
    ads["uchar_vector_port3"] = val
    retrieved_val4 = ads.get_port_data("uchar_vector_port2")
    retrieved_val5 = ads.value("uchar_vector_port3")
    retrieved_val6 = ads.value_or("uchar_vector_port3")
    retrieved_val7 = ads["uchar_vector_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)


def test_value_or_cpp_types():
    from kwiver.sprokit.adapters import adapter_data_set
    import kwiver.vital.types as kvt

    ads = adapter_data_set.AdapterDataSet.create()
    val = adapter_data_set.VectorDouble([3.14, 4.14])
    # Typed
    retrieved_val1 = ads._value_or_double_vector("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_is_not_none(ads._value_or_double_vector("nonexistant_port"))

    val = adapter_data_set.VectorString(["s00", "s01"])
    # Typed
    retrieved_val1 = ads._value_or_string_vector("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_is_not_none(ads._value_or_string_vector("nonexistant_port"))

    val = adapter_data_set.VectorUChar([100, 101])
    # Typed
    retrieved_val1 = ads._value_or_uchar_vector("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(str(val), str(retrieved_val1))
    err_ne(str(val), str(retrieved_val2))
    err_is_not_none(ads._value_or_uchar_vector("nonexistant_port"))


# Now try creating datums of these bound types
def test_add_get_cpp_types_with_datum():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()

    val = datum.VectorDouble([6.3, 8.9])
    d = datum.new_double_vector(val)
    ads.add_datum("datum_double_vector_port", d)
    retrieved_val1 = ads._get_port_data_double_vector(
        "datum_double_vector_port"
    )
    retrieved_val2 = ads._value_double_vector("datum_double_vector_port")
    retrieved_val3 = ads._value_or_double_vector("datum_double_vector_port")
    retrieved_val4 = ads.get_port_data("datum_double_vector_port")
    retrieved_val5 = ads.value("datum_double_vector_port")
    retrieved_val6 = ads.value_or("datum_double_vector_port")
    retrieved_val7 = ads["datum_double_vector_port"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = datum.VectorString(["foo", "bar"])
    d = datum.new_string_vector(val)
    ads.add_datum("datum_string_vector_port", d)
    retrieved_val1 = ads._get_port_data_string_vector(
        "datum_string_vector_port"
    )
    retrieved_val2 = ads._value_string_vector("datum_string_vector_port")
    retrieved_val3 = ads._value_or_string_vector("datum_string_vector_port")
    retrieved_val4 = ads.get_port_data("datum_string_vector_port")
    retrieved_val5 = ads.value("datum_string_vector_port")
    retrieved_val6 = ads.value_or("datum_string_vector_port")
    retrieved_val7 = ads["datum_string_vector_port"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = datum.VectorUChar([102, 103])
    d = datum.new_uchar_vector(val)
    ads.add_datum("datum_uchar_vector_port", d)
    retrieved_val1 = ads._get_port_data_uchar_vector("datum_uchar_vector_port")
    retrieved_val2 = ads._value_uchar_vector("datum_uchar_vector_port")
    retrieved_val3 = ads._value_or_uchar_vector("datum_uchar_vector_port")
    retrieved_val4 = ads.get_port_data("datum_uchar_vector_port")
    retrieved_val5 = ads.value("datum_uchar_vector_port")
    retrieved_val6 = ads.value_or("datum_uchar_vector_port")
    retrieved_val7 = ads["datum_uchar_vector_port"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)


# Next kwiver vital types
def test_add_get_vital_types():
    from kwiver.vital import types as kvt
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()

    val = kvt.BoundingBoxD(1, 1, 2, 2)
    # Typed methods
    ads._add_bounding_box("bounding_box_port", val)
    retrieved_val1 = ads._get_port_data_bounding_box("bounding_box_port")
    retrieved_val2 = ads._value_bounding_box("bounding_box_port")
    retrieved_val3 = ads._value_or_bounding_box("bounding_box_port")

    # Non typed methods
    ads.add_value("bounding_box_port2", val)
    ads["bounding_box_port3"] = val
    retrieved_val4 = ads.get_port_data("bounding_box_port2")
    retrieved_val5 = ads.value("bounding_box_port3")
    retrieved_val6 = ads.value_or("bounding_box_port3")
    retrieved_val7 = ads["bounding_box_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = kvt.Timestamp(100, 1)
    # Typed methods
    ads._add_timestamp("timestamp_port", val)
    retrieved_val1 = ads._get_port_data_timestamp("timestamp_port")
    retrieved_val2 = ads._value_timestamp("timestamp_port")
    retrieved_val3 = ads._value_or_timestamp("timestamp_port")

    # Non typed methods
    ads.add_value("timestamp_port2", val)
    ads["timestamp_port3"] = val
    retrieved_val4 = ads.get_port_data("timestamp_port2")
    retrieved_val5 = ads.value("timestamp_port3")
    retrieved_val6 = ads.value_or("timestamp_port3")
    retrieved_val7 = ads["timestamp_port3"]
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_ne(val, retrieved_val3)
    err_ne(val, retrieved_val4)
    err_ne(val, retrieved_val5)
    err_ne(val, retrieved_val6)
    err_ne(val, retrieved_val7)

    val = kvt.F2FHomography(1)
    # Typed methods
    ads._add_f2f_homography("f2f_homography_port", val)
    retrieved_val1 = ads._get_port_data_f2f_homography("f2f_homography_port")
    retrieved_val2 = ads._value_f2f_homography("f2f_homography_port")
    retrieved_val3 = ads._value_or_f2f_homography("f2f_homography_port")

    # Non typed methods
    ads.add_value("f2f_homography_port2", val)
    ads["f2f_homography_port3"] = val
    retrieved_val4 = ads.get_port_data("f2f_homography_port2")
    retrieved_val5 = ads.value("f2f_homography_port3")
    retrieved_val6 = ads.value_or("f2f_homography_port3")
    retrieved_val7 = ads["f2f_homography_port3"]
    err_ne(str(val), str(retrieved_val1))
    err_ne(str(val), str(retrieved_val2))
    err_ne(str(val), str(retrieved_val3))
    err_ne(str(val), str(retrieved_val4))
    err_ne(str(val), str(retrieved_val5))
    err_ne(str(val), str(retrieved_val6))
    err_ne(str(val), str(retrieved_val7))


def test_value_or_vital_types():
    from kwiver.sprokit.adapters import adapter_data_set
    import kwiver.vital.types as kvt

    ads = adapter_data_set.AdapterDataSet.create()
    val = kvt.BoundingBoxD(1, 1, 2, 2)
    # Typed
    retrieved_val1 = ads._value_or_bounding_box("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_is_not_none(ads._value_or_bounding_box("nonexistant_port"))

    val = kvt.Timestamp(100, 1)
    # Typed
    retrieved_val1 = ads._value_or_timestamp("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(val, retrieved_val1)
    err_ne(val, retrieved_val2)
    err_is_not_none(ads._value_or_timestamp("nonexistant_port"))

    val = kvt.F2FHomography(1)
    # Typed
    retrieved_val1 = ads._value_or_f2f_homography("nonexistant_port", val)

    # Non typed
    retrieved_val2 = ads.value_or("nonexistant_port", val)
    err_ne(str(val), str(retrieved_val1))
    err_ne(str(val), str(retrieved_val2))
    err_is_not_none(ads._value_or_f2f_homography("nonexistant_port"))


# Now test overwriting
def test_overwrite():
    from kwiver.vital import types as kvt
    from kwiver.sprokit.adapters import adapter_data_set

    OVERWRITE_PORT = "test_overwrite_port"
    ads = adapter_data_set.AdapterDataSet.create()

    # Overwriting with same datum
    ads.add_datum(OVERWRITE_PORT, datum.new("d2"))
    overwrite_helper(
        ads.add_datum,
        ads.get_port_data,
        datum.new("d3"),
        "datum_string",
        OVERWRITE_PORT,
    )

    # Overwriting with completely different types
    overwrite_helper(
        ads.add_datum,
        ads.get_port_data,
        datum.new(12),
        "datum_int",
        OVERWRITE_PORT,
    )
    overwrite_helper(
        ads._add_string_vector,
        ads._get_port_data_string_vector,
        adapter_data_set.VectorString(["baz", "qux"]),
        "string_vector",
        OVERWRITE_PORT,
    )
    overwrite_helper(
        ads._add_timestamp,
        ads._get_port_data_timestamp,
        kvt.Timestamp(100, 10),
        "timestamp",
        OVERWRITE_PORT,
    )
    overwrite_helper(
        ads.add_value, ads.get_port_data, 15, "int", OVERWRITE_PORT
    )
    overwrite_helper(
        ads._add_double_vector,
        ads._get_port_data_double_vector,
        adapter_data_set.VectorDouble([4, 8]),
        "double_vector",
        OVERWRITE_PORT,
    )


# Want to make sure data inside a datum created with the automatic
# conversion constructor can be retrieved with a type specific getter, and
# vice versa
def test_mix_add_and_get():
    from kwiver.sprokit.adapters import adapter_data_set
    from kwiver.vital import types as kvt

    ads = adapter_data_set.AdapterDataSet.create()

    # Try adding with generic adder first, retrieving with
    # type specific get function
    ads["string_port"] = "string_value"
    err_ne(
        ads._get_port_data_string("string_port"),
        "string_value",
    )

    ads["timestamp_port"] = kvt.Timestamp(1000000000, 10)
    err_ne(
        ads._get_port_data_timestamp("timestamp_port"),
        kvt.Timestamp(1000000000, 10),
    )

    ads["vector_string_port"] = adapter_data_set.VectorString(
        ["element1", "element2"]
    )
    err_ne(
        ads._get_port_data_string_vector("vector_string_port"),
        adapter_data_set.VectorString(["element1", "element2"]),
    )

    # Now try the opposite
    ads._add_string("string_port", "string_value")
    err_ne(ads["string_port"], "string_value")

    ads._add_timestamp("timestamp_port", kvt.Timestamp(1000000000, 10))
    err_ne(ads["timestamp_port"], kvt.Timestamp(1000000000, 10))

    ads._add_string_vector(
        "vector_string_port",
        adapter_data_set.VectorString(["element1", "element2"]),
    )
    err_ne(
        ads["vector_string_port"],
        adapter_data_set.VectorString(["element1", "element2"]),
    )


# Make sure that None isn't acceptable, even for pointers
def test_add_none():
    from kwiver.sprokit.adapters import adapter_data_set
    from kwiver.vital import types as kvt

    ads = adapter_data_set.AdapterDataSet.create()

    expect_exception(
        "attempting to store None as a string vector",
        TypeError,
        ads._add_string_vector,
        "none_vector_string_port",
        None,
    )

    expect_exception(
        "attempting to store None as a track set",
        TypeError,
        ads._add_track_set,
        "none_track_set_port",
        None,
    )

    expect_exception(
        "attempting to store None as a timestamp",
        TypeError,
        ads._add_timestamp,
        "none_timestamp_port",
        None,
    )

    # Should also fail for the automatic type conversion
    expect_exception(
        "attempting to store none through automatic conversion",
        TypeError,
        ads.add_value,
        "none_port",
        None,
    )


def _create_ads():
    from kwiver.vital import types as kvt
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()

    # Construct a few elements
    ads["string_port"] = "string_value"
    ads["timestamp_port"] = kvt.Timestamp(1000000000, 10)
    ads["vector_string_port"] = adapter_data_set.VectorString(
        ["element1", "element2"]
    )

    return ads


def test_iter():
    from kwiver.vital import types as kvt
    from kwiver.sprokit.adapters import adapter_data_set

    ads = _create_ads()

    for port, dat in ads:
        if port == "string_port":
            if dat.get_datum() != "string_value":
                test_error(
                    "Didn't retrieve correct string value on first iteration"
                )
        elif port == "timestamp_port":
            if dat.get_datum() != kvt.Timestamp(1000000000, 10):
                test_error(
                    "Didn't retrieve correct timestamp value on second iteration"
                )
        elif port == "vector_string_port":
            if dat.get_datum() != datum.VectorString(["element1", "element2"]):
                test_error(
                    "Didn't retrieve correct string vector on third iteration"
                )
        else:
            test_error("unknown port: {}".format(port))


def check_formatting_fxn(exp, act, fxn_name):
    if not act == exp:
        test_error(
            "Expected {} to return '{}'. Got '{}'".format(fxn_name, exp, act)
        )

    print(act)


def test_nice():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()
    check_formatting_fxn("size=0", ads.__nice__(), "__nice__")

    ads = _create_ads()
    check_formatting_fxn("size=3", ads.__nice__(), "__nice__")


def test_repr():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()
    exp = "<AdapterDataSet(size=0) at {}>".format(hex(id(ads)))
    check_formatting_fxn(exp, ads.__repr__(), "__repr__")

    ads = _create_ads()
    exp = "<AdapterDataSet(size=3) at {}>".format(hex(id(ads)))
    check_formatting_fxn(exp, ads.__repr__(), "__repr__")


def test_str():
    from kwiver.sprokit.adapters import adapter_data_set

    # Formatted string we'll fill in for each ads below
    exp_stem = "<AdapterDataSet(size={})>\n\t{{{}}}"

    ads = adapter_data_set.AdapterDataSet.create()
    check_formatting_fxn(exp_stem.format(0, ""), ads.__str__(), "__str__")

    ads = _create_ads()
    # This one actually has content, so we'll have to manually derive it
    content = ""
    content += "string_port: " + str(ads["string_port"])
    content += ", timestamp_port: " + str(ads["timestamp_port"])
    content += ", vector_string_port: " + str(ads["vector_string_port"])
    check_formatting_fxn(exp_stem.format(3, content), ads.__str__(), "__str__")


def test_len():
    from kwiver.sprokit.adapters import adapter_data_set

    ads = adapter_data_set.AdapterDataSet.create()

    # Check initial
    if len(ads) != 0:
        test_error(
            "adapter_data_set with 0 values returned size {}".format(len(ads))
        )

    ads = _create_ads()

    if len(ads) != 3:
        test_error(
            "adapter_data_set with 3 values returned size {}".format(len(ads))
        )


if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        test_error("Expected two arguments")
        sys.exit(1)

    testname = sys.argv[1]

    run_test(testname, find_tests(locals()))
