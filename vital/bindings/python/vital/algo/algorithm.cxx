/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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

#include <pybind11/pybind11.h>

#include <vital/algo/algorithm.h>

namespace py = pybind11;

typedef kwiver::vital::algorithm kv_algorithm;
typedef std::shared_ptr<kv_algorithm> kv_algorithm_t;

// Trampoline class to allow us to use virtual methods
class algo_trampoline
  : public kv_algorithm
{
  public:

    using kv_algorithm::algorithm;
    std::string type_name() const override;
    kwiver::vital::config_block_sptr get_configuration() const override;
    void set_configuration(kwiver::vital::config_block_sptr config) override;
    bool check_configuration(kwiver::vital::config_block_sptr config) const override;
};

std::string
algo_trampoline
::type_name() const
{
  PYBIND11_OVERLOAD_PURE(
    std::string,
    kv_algorithm,
    type_name,
  );
}

kwiver::vital::config_block_sptr
algo_trampoline
::get_configuration() const
{
  PYBIND11_OVERLOAD(
    kwiver::vital::config_block_sptr,
    kv_algorithm,
    get_configuration,
  );
}

void
algo_trampoline
::set_configuration(kwiver::vital::config_block_sptr config)
{
  PYBIND11_OVERLOAD_PURE(
    void,
    kv_algorithm,
    set_configuration,
    config
  );
}

bool
algo_trampoline
::check_configuration(kwiver::vital::config_block_sptr config) const
{
  PYBIND11_OVERLOAD_PURE(
    bool,
    kv_algorithm,
    check_configuration,
    config
  );
}

PYBIND11_MODULE(algorithm, m)
{

  py::class_<kv_algorithm, algo_trampoline, kv_algorithm_t >(m, "Algorithm")
  .def(py::init<>())
  .def("type_name", &kv_algorithm::type_name, py::call_guard<py::gil_scoped_release>(),
    "Return the name of the base algorithm.")
  .def("impl_name", &kv_algorithm::impl_name, py::call_guard<py::gil_scoped_release>(),
    "Return the name of this implementation.")
  .def("get_configuration", &kv_algorithm::get_configuration, py::call_guard<py::gil_scoped_release>(),
    "Return this algorithm's configuration block.")
  .def("set_configuration", &kv_algorithm::set_configuration, py::call_guard<py::gil_scoped_release>(),
    py::arg("config"),
    "Set this algorithm's properties with a configuration block.")
  .def("check_configuration", &kv_algorithm::check_configuration, py::call_guard<py::gil_scoped_release>(),
    py::arg("config"),
    "Check to see if the provided configuration block is valid.")
  .def_static("get_nested_algo_configuration", &kv_algorithm::get_nested_algo_configuration, py::call_guard<py::gil_scoped_release>(),
    py::arg("type_name"), py::arg("name"), py::arg("config"), py::arg("nested_algo"),
    "Get a nested algorithm's configuration block.")
  .def_static("set_nested_algo_configuration", &kv_algorithm::set_nested_algo_configuration, py::call_guard<py::gil_scoped_release>(),
    py::arg("type_name"), py::arg("name"), py::arg("config"), py::arg("nested_algo"),
    "Set a nested algorithm's configuration block.")
  .def_static("check_nested_algo_configuration", &kv_algorithm::check_nested_algo_configuration, py::call_guard<py::gil_scoped_release>(),
    py::arg("type_name"), py::arg("name"), py::arg("config"),
    "Check if a nested algorithm's configuration block is valid.")
  .def("set_impl_name", &kv_algorithm::set_impl_name, py::call_guard<py::gil_scoped_release>(),
    py::arg("name"),
    "Set the name of this implementation.")
  ;
}
