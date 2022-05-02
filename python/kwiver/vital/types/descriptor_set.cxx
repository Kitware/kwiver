// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>

#include <vital/types/descriptor_set.h>

#include <memory>

namespace py = pybind11;
namespace kwiver {
namespace vital  {
namespace python {

typedef kwiver::vital::descriptor_set desc_set;
typedef kwiver::vital::simple_descriptor_set s_desc_set;

std::shared_ptr<s_desc_set>
new_desc_set()
{
  return std::make_shared<s_desc_set>();
}

std::shared_ptr<s_desc_set>
new_desc_set1(py::list py_list)
{
  std::vector<std::shared_ptr<kwiver::vital::descriptor>> desc_list;
  for(auto py_desc : py_list)
  {
    desc_list.push_back(py_desc.cast<std::shared_ptr<kwiver::vital::descriptor>>());
  }
  return std::make_shared<s_desc_set>(desc_list);
}
}
}
}

using namespace kwiver::vital::python;
PYBIND11_MODULE(descriptor_set, m)
{
  py::class_<desc_set, std::shared_ptr<desc_set>>(m, "BaseDescriptorSet")
  .def("descriptors", &desc_set::descriptors)
  .def("empty", &desc_set::empty)
  .def("size", &desc_set::size)
  .def("__len__", &desc_set::size)
  .def("__getitem__", static_cast<kwiver::vital::descriptor_sptr (desc_set::*)(size_t)>(&desc_set::at))
  ;

  py::class_<s_desc_set, desc_set, std::shared_ptr<s_desc_set>>(m, "DescriptorSet")
  .def(py::init(&new_desc_set))
  .def(py::init(&new_desc_set1),
    py::arg("list"))
  ;

}
