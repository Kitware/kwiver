/*ckwg +29
 * Copyright 2011-2013 by Kitware, Inc.
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

#include "PyProcess.cxx"

#include <sprokit/python/util/python_exceptions.h>
#include <sprokit/python/util/python_gil.h>

#include <pybind11/stl_bind.h>
#include <pybind11/operators.h>

#include <utility>

/**
 * \file process.cxx
 *
 * \brief Python bindings for \link sprokit::process\endlink.
 */

using namespace pybind11;

PYBIND11_MODULE(process, m)
{

  bind_vector<std::vector<sprokit::process::name_t> >(m, "ProcessNames"
    , module_local()
    , "A collection of process names.")
  ;
  m.attr("ProcessTypes") = m.attr("ProcessNames");
  m.attr("ProcessTypes").attr("__doc__") = "A collection of process types.";

  class_<std::set<std::string> >(m, "ProcessProperties"
    , module_local()
    , "A collection of properties on a process.")
    .def(init<>());
  ;

  m.attr("Ports") = m.attr("ProcessNames");
  m.attr("Ports").attr("__doc__") = "A collection of ports.";

  m.attr("PortFlags") = m.attr("ProcessProperties");
  m.attr("PortFlags").attr("__doc__") = "A collection of port flags.";

  class_<sprokit::process::port_frequency_t>(m, "PortFrequency"
    , "A frequency for a port.")
    .def(init<sprokit::process::frequency_component_t>())
    .def(init<sprokit::process::frequency_component_t, sprokit::process::frequency_component_t>())
    .def("numerator", &sprokit::process::port_frequency_t::numerator
      , "The numerator of the frequency.")
    .def("denominator", &sprokit::process::port_frequency_t::denominator
      , "The denominator of the frequency.")
    .def(self <  self)
    .def(self <= self)
    .def(self == self)
    .def(self >= self)
    .def(self >  self)
    .def(self + self)
    .def(self - self)
    .def(self * self)
    .def(self / self)
    .def(!self)
  ;
  class_<sprokit::process::port_addr_t>(m, "PortAddr"
    , "An address for a port within a pipeline.")
    .def(init<>())
  ;
  bind_vector<sprokit::process::port_addrs_t >(m, "PortAddrs"
    , "A collection of port addresses.")
  ;
  class_<sprokit::process::connection_t>(m, "Connection"
    , "A connection between two ports.")
    .def(init<>());
  ;
  bind_vector<sprokit::process::connections_t >(m, "Connections"
    , "A collection of connections.")
  ;

  class_<sprokit::process::port_info>(m, "PortInfo"
    , "Information about a port on a process.")
    .def(init<sprokit::process::port_type_t, sprokit::process::port_flags_t, sprokit::process::port_description_t, sprokit::process::port_frequency_t>())
    .def_readonly("type", &sprokit::process::port_info::type)
    .def_readonly("flags", &sprokit::process::port_info::flags)
    .def_readonly("description", &sprokit::process::port_info::description)
    .def_readonly("frequency", &sprokit::process::port_info::frequency)
  ;

  class_<sprokit::process::conf_info>(m, "ConfInfo"
    , "Information about a configuration on a process.")
    .def(init<kwiver::vital::config_block_value_t, kwiver::vital::config_block_description_t, bool>())
    .def_readonly("default", &sprokit::process::conf_info::def)
    .def_readonly("description", &sprokit::process::conf_info::description)
    .def_readonly("tunable", &sprokit::process::conf_info::tunable)
  ;

  class_<sprokit::process::data_info>(m, "DataInfo"
    , "Information about a set of data packets from edges.")
    .def(init<bool, sprokit::datum::type_t>())
    .def_readonly("in_sync", &sprokit::process::data_info::in_sync)
    .def_readonly("max_status", &sprokit::process::data_info::max_status)
  ;

  enum_<sprokit::process::data_check_t>(m, "DataCheck"
    , "Levels of input validation")
    .value("none", sprokit::process::check_none)
    .value("sync", sprokit::process::check_sync)
    .value("valid", sprokit::process::check_valid)
  ;

  class_<PyProcess>(m, "PythonProcess"
    , "The base class for Python processes.")
    .def(init<kwiver::vital::config_block_sptr>())
  ;

}

