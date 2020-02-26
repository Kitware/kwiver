/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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

#include <vital/types/landmark.h>

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <python/kwiver/vital/types/landmark.h>


std::shared_ptr<PyLandmarkBase>
new_landmark(py::object loc_obj, py::object scale_obj, char ctype)
{
  std::shared_ptr<PyLandmarkBase> retVal;
  if(ctype == 'd')
  {
    // Get our arguments, taking care of default cases
    Eigen::Matrix<double, 3, 1> loc = Eigen::Matrix<double, 3, 1>();
    loc << 0,0,0;
    if(!loc_obj.is(py::none()))
    {
      loc = loc_obj.cast<Eigen::Matrix<double, 3, 1>>();
    }

    double scale = 1.;
    if(!scale_obj.is(py::none()))
    {
      scale = scale_obj.cast<double>();
    }

    retVal = std::shared_ptr<PyLandmarkBase>(new PyLandmarkd(loc, scale));
  }
  else if(ctype == 'f')
  {
    // Get our arguments, taking care of default cases
    Eigen::Matrix<float, 3, 1> loc = Eigen::Matrix<float, 3, 1>();
    loc << 0,0,0;
    if(!loc_obj.is(py::none()))
    {
      loc = loc_obj.cast<Eigen::Matrix<float, 3, 1>>();
    }

    float scale = 1.;
    if(!scale_obj.is(py::none()))
    {
      scale = scale_obj.cast<float>();
    }

    retVal = std::shared_ptr<PyLandmarkBase>(new PyLandmarkf(loc, scale));
  }

  return retVal;
}

void landmark(py::module &m)
{

  py::class_<PyLandmarkBase, std::shared_ptr<PyLandmarkBase>>(m, "Landmark")
  .def(py::init(&new_landmark),
    py::arg("loc")=py::none(), py::arg("scale")=py::none(), py::arg("ctype")='d')
  .def_property_readonly("type_name", &PyLandmarkBase::get_type)
  .def_property("loc", &PyLandmarkBase::get_loc, &PyLandmarkBase::set_loc)
  .def_property("scale", &PyLandmarkBase::get_scale, &PyLandmarkBase::set_scale)
  .def_property("normal", &PyLandmarkBase::get_normal, &PyLandmarkBase::set_normal)
  .def_property("covariance", &PyLandmarkBase::get_covariance, &PyLandmarkBase::set_covariance)
  .def_property("color", &PyLandmarkBase::get_color, &PyLandmarkBase::set_color)
  .def_property("observations", &PyLandmarkBase::get_observations, &PyLandmarkBase::set_observations)
  ;

}
