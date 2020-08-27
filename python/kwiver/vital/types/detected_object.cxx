/*ckwg +29
 * Copyright 2017-2020 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <vital/types/detected_object.h>

#include <pybind11/eigen.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdexcept>

typedef kwiver::vital::detected_object det_obj;

namespace py = pybind11;
namespace kwiver {
namespace vital {
namespace python {

// We want to be able to add a mask in the python constructor
// so we need a pass-through cstor
std::shared_ptr<det_obj>
new_detected_object(bounding_box<double> bbox,
                    double conf,
                    detected_object_type_sptr type,
                    image_container_sptr mask)
{
  std::shared_ptr<det_obj> new_obj(new det_obj(bbox, conf, type));

  if(mask)
  {
    new_obj->set_mask(mask);
  }

  return new_obj;
}

// Pybind casts away all const-ness, and since a few getters/setters
// in the detected_object class have pointers to const, we have to copy
// them in order to avoid undefined behavior.
descriptor_sptr
det_obj_const_safe_descriptor(detected_object const& self)
{
  auto desc = self.descriptor();
  if (desc)
  {
    // Create a pointer to a copy so we don't violate const
    return desc->clone();
  }
  return nullptr;
}

void
det_obj_const_safe_set_descriptor(detected_object& self, descriptor_sptr desc)
{
  if (desc)
  {
    // Return a pointer to a copy
    // clone() returns pointer to base
    auto cloned_desc = desc->clone();
    auto des_dyn_sptr = std::dynamic_pointer_cast<descriptor_dynamic<double>>(cloned_desc);

    // Check conversion worked
    if (!des_dyn_sptr)
    {
      throw std::runtime_error("Downcasting descriptor_dynamic<double> from base pointer failed");
    }
    self.set_descriptor(des_dyn_sptr);
  }
  else
  {
    self.set_descriptor(nullptr);
  }
}


// TODO: uncomment these when rebased on latest master with metadata API changes
// Those changes will make copying metadata objects much easier.
// metadata_sptr
// copy_metadata(metadata_sptr m)
// {
//   metadata m_cpy(m);
// }

// image_container_sptr
// det_obj_const_safe_mask(detected_object const& self)
// {
//   auto mask = self.mask();
//   if (mask)
//   {
//     // Create a pointer to a copy so we don't violate const
//     // TODO
//   }
//   return nullptr;
// }

// void
// det_obj_const_safe_set_mask(detected_object& self, descriptor_sptr desc)
// {
//   if (desc)
//   {
//     // Return a pointer to a copy
//     // clone() returns pointer to base
//     auto cloned_desc = desc->clone();
//     auto des_dyn_sptr = std::dynamic_pointer_cast<descriptor_dynamic<double>>(cloned_desc);

//     // Check conversion worked
//     if (!des_dyn_sptr)
//     {
//       throw std::runtime_error("Downcasting descriptor_dynamic<double> from base pointer failed");
//     }
//     self.set_descriptor(des_dyn_sptr);
//   }
//   else
//   {
//     self.set_descriptor(nullptr);
//   }
// }

}
}
}

using namespace kwiver::vital;

PYBIND11_MODULE(detected_object, m)
{
  /*
   *
    Developer:
        python -c "import vital.types; help(vital.types.DetectedObject)"
        python -m xdoctest vital.types DetectedObject --xdoc-dynamic
   *
   */

  py::class_<det_obj, std::shared_ptr<det_obj>>(m, "DetectedObject", R"(
    Represents a detected object within an image

    Example:
        >>> from kwiver.vital.types import *
        >>> from PIL import Image as PILImage
        >>> from kwiver.vital.util import VitalPIL
        >>> import numpy as np
        >>> bbox = BoundingBox(0, 10, 100, 50)
        >>> # Construct an object without a mask
        >>> dobj1 = DetectedObject(bbox, 0.2)
        >>> assert dobj1.mask is None
        >>> # Construct an object with a mask
        >>> pil_img = PILImage.fromarray(np.zeros((10, 10), dtype=np.uint8))
        >>> vital_img = VitalPIL.from_pil(pil_img)
        >>> mask = ImageContainer(vital_img)
        >>> self = DetectedObject(bbox, 1.0, mask=mask)
        >>> assert self.mask is mask
        >>> print(self)
        <DetectedObject(conf=1.0)>
    )")
  .def(py::init(&python::new_detected_object),
    py::arg("bbox"), py::arg("confidence")=1.0,
    py::arg("classifications")=detected_object_type_sptr(),
    py::arg("mask")=image_container_sptr(), py::doc(R"(
      Args:
          bbox: coarse localization of the object in image coordinates
          confidence: confidence in this detection (default=1.0)
          classifications: optional object classification (default=None)
    ")"))
  .def("__nice__", [](det_obj& self) -> std::string {
    auto locals = py::dict(py::arg("self")=self);
    py::exec(R"(
        retval = 'conf={}'.format(self.confidence)
    )", py::globals(), locals);
    return locals["retval"].cast<std::string>();
    })
  .def("__repr__", [](py::object& self) -> std::string {
    auto locals = py::dict(py::arg("self")=self);
    py::exec(R"(
        classname = self.__class__.__name__
        devnice = self.__nice__()
        retval = '<%s(%s) at %s>' % (classname, devnice, hex(id(self)))
    )", py::globals(), locals);
    return locals["retval"].cast<std::string>();
    })
  .def("__str__", [](py::object& self) -> std::string {
    auto locals = py::dict(py::arg("self")=self);
    py::exec(R"(
        classname = self.__class__.__name__
        devnice = self.__nice__()
        retval = '<%s(%s)>' % (classname, devnice)
    )", py::globals(), locals);
    return locals["retval"].cast<std::string>();
    })
  .def("clone", &det_obj::clone)
  .def("add_note", &det_obj::add_note)
  .def("clear_notes", &det_obj::clear_notes)
  .def("add_keypoint", &det_obj::add_keypoint)
  .def("clear_keypoints", &det_obj::clear_keypoints)
  // TODO: Uncomment after above const-safe methods are implemented for mask
  // .def_property("mask", &det_obj::mask, &det_obj::set_mask)

  // Convey that users can't access the the underlying descriptor directly.
  // Must go through the setter. This is because of the const-issue discussed above.
  .def("descriptor_copy", &python::det_obj_const_safe_descriptor)
  .def("set_descriptor", &python::det_obj_const_safe_set_descriptor)
  .def_property("bounding_box", &det_obj::bounding_box, &det_obj::set_bounding_box)
  .def_property("geo_point", &det_obj::geo_point, &det_obj::set_geo_point)
  .def_property("confidence", &det_obj::confidence, &det_obj::set_confidence)
  .def_property("index", &det_obj::index, &det_obj::set_index)
  .def_property("detector_name", &det_obj::detector_name, &det_obj::set_detector_name)
  .def_property("type", &det_obj::type, &det_obj::set_type)
  .def_property_readonly("notes", &det_obj::notes)
  .def_property_readonly("keypoints", &det_obj::keypoints)
  ;
}
