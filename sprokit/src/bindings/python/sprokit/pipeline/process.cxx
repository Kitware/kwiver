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
  ;
  class_<sprokit::process::port_addrs_t>(m, "PortAddrs"
    , "A collection of port addresses.")
  ;
  class_<sprokit::process::connection_t>(m, "Connection"
    , "A connection between two ports.")
  ;
  class_<sprokit::process::connections_t>(m, "Connections"
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
    .def("configure", &sprokit::process::configure
      , "Configure the process.")
    .def("init", &sprokit::process::init
      , "Initializes the process.")
    .def("reset", &sprokit::process::reset
      , "Resets the process.")
    .def("step", &sprokit::process::step
      , "Steps the process for one iteration.")
    .def("properties", &sprokit::process::properties
      , "Returns the properties on the process.")
    .def("connect_input_port", &sprokit::process::connect_input_port
      , arg("port"), arg("edge")
      , "Connects the given edge to the input port.")
    .def("connect_output_port", &sprokit::process::connect_output_port
      , arg("port"), arg("edge")
      , "Connects the given edge to the output port.")
    .def("input_ports", &sprokit::process::input_ports
      , "Returns a list of input ports on the process.")
    .def("output_ports", &sprokit::process::output_ports
      , "Returns a list of output ports on the process.")
    .def("input_port_info", &sprokit::process::input_port_info
      , (arg("port"))
      , "Returns information about the given input port.")
    .def("output_port_info", &sprokit::process::output_port_info
      , (arg("port"))
      , "Returns information about the given output port.")
    .def("set_input_port_type", &sprokit::process::set_input_port_type
      , arg("port"), arg("new_type")
      , "Sets the type for an input port.")
    .def("set_output_port_type", &sprokit::process::set_output_port_type
      , arg("port"), arg("new_type")
      , "Sets the type for an output port.")
    .def("available_config", &sprokit::process::available_config
      , "Returns a list of available configuration keys for the process.")
    .def("available_tunable_config", &sprokit::process::available_tunable_config
      , "Returns a list of available tunable configuration keys for the process.")
    .def("config_info", &sprokit::process::config_info
      , (arg("config"))
      , "Returns information about the given configuration key.")
    .def("name", &sprokit::process::name
      , "Returns the name of the process.")
    .def("type", &sprokit::process::type
      , "Returns the type of the process.")
    .def_readonly_static("property_no_threads", &sprokit::process::property_no_threads)
    .def_readonly_static("property_no_reentrancy", &sprokit::process::property_no_reentrancy)
    .def_readonly_static("property_unsync_input", &sprokit::process::property_unsync_input)
    .def_readonly_static("property_unsync_output", &sprokit::process::property_unsync_output)
    .def_readonly_static("port_heartbeat", &sprokit::process::port_heartbeat)
    .def_readonly_static("config_name", &sprokit::process::config_name)
    .def_readonly_static("config_type", &sprokit::process::config_type)
    .def_readonly_static("type_any", &sprokit::process::type_any)
    .def_readonly_static("type_none", &sprokit::process::type_none)
    .def_readonly_static("type_data_dependent", &sprokit::process::type_data_dependent)
    .def_readonly_static("type_flow_dependent", &sprokit::process::type_flow_dependent)
    .def_readonly_static("flag_output_const", &sprokit::process::flag_output_const)
    .def_readonly_static("flag_output_shared", &sprokit::process::flag_output_shared)
    .def_readonly_static("flag_input_static", &sprokit::process::flag_input_static)
    .def_readonly_static("flag_input_mutable", &sprokit::process::flag_input_mutable)
    .def_readonly_static("flag_input_nodep", &sprokit::process::flag_input_nodep)
    .def_readonly_static("flag_required", &sprokit::process::flag_required)
    .def("_configure", &PyProcess::_configure
      , "Configures the process subclass.")
    .def("_init", &PyProcess::_init
      , "Initializes the process subclass.")
    .def("_reset", &PyProcess::_reset
      , "Resets the process subclass.")
    .def("_flush", &PyProcess::_flush
      , "Flushes the process subclass.")
    .def("_step", &PyProcess::_step
      , "Step the process subclass for one iteration.")
    .def("_reconfigure", &PyProcess::_reconfigure
      , (arg("config"))
      , "Runtime configuration for subclasses.")
    .def("_properties", &PyProcess::_properties
      , "The properties on the subclass.")
    .def("_input_ports", &PyProcess::_input_ports
      , "Returns a list on input ports on the subclass process.")
    .def("_output_ports", &PyProcess::_output_ports
      , "Returns a list on output ports on the subclass process.")
    .def("_input_port_info", &PyProcess::_input_port_info
      , (arg("port"))
      , "Returns information about the given subclass input port.")
    .def("_output_port_info", &PyProcess::_output_port_info
      , (arg("port"))
      , "Returns information about the given subclass output port.")
    .def("_set_input_port_type", &PyProcess::_set_input_port_type
      , arg("port"), arg("new_type")
      , "Sets the type for an input port.")
    .def("_set_output_port_type", &PyProcess::_set_output_port_type
      , arg("port"), arg("new_type")
      , "Sets the type for an output port.")
    .def("_available_config", &PyProcess::_available_config
      , "Returns a list of available configuration keys for the subclass process.")
    .def("_config_info", &PyProcess::_config_info
      , (arg("key"))
      , "Returns information about the given configuration key.")
    .def("declare_input_port", &PyProcess::_declare_input_port
      , arg("port"), arg("info")
      , "Declare an input port on the process.")
    .def("declare_input_port", &PyProcess::_declare_input_port_1
      , arg("port"), arg("type"), arg("flags"), arg("description"), arg("frequency") = sprokit::process::port_frequency_t(1)
      , "Declare an input port on the process.")
    .def("declare_output_port", &PyProcess::_declare_output_port
      , arg("port"), arg("info")
      , "Declare an output port on the process.")
    .def("declare_output_port", &PyProcess::_declare_output_port_1
      , arg("port"), arg("type"), arg("flags"), arg("description"), arg("frequency") = sprokit::process::port_frequency_t(1)
      , "Declare an output port on the process.")
    .def("set_input_port_frequency", &PyProcess::_set_input_port_frequency
      , arg("port"), arg("new_frequency")
      , "Set an input port\'s frequency")
    .def("set_output_port_frequency", &PyProcess::_set_output_port_frequency
      , arg("port"), arg("new_frequency")
      , "Set an output port\'s frequency")
    .def("remove_input_port", &PyProcess::_remove_input_port
      , (arg("port"))
      , "Remove an input port from the process.")
    .def("remove_output_port", &PyProcess::_remove_output_port
      , (arg("port"))
      , "Remove an output port from the process.")
    .def("declare_configuration_key", &PyProcess::_declare_configuration_key
      , arg("key"), arg("info")
      , "Declare a configuration key for the process.")
    .def("declare_configuration_key", &PyProcess::_declare_configuration_key_1
      , arg("key"), arg("default"), arg("description")
      , "Declare a configuration key for the process.")
    .def("declare_configuration_key", &PyProcess::_declare_configuration_key_2
      , arg("key"), arg("default"), arg("description"), arg("tunable")
      , "Declare a configuration key for the process.")
    .def("mark_process_as_complete", &PyProcess::_mark_process_as_complete
      , "Tags the process as complete.")
    .def("has_input_port_edge", &PyProcess::_has_input_port_edge
      , (arg("port"))
      , "True if there is an edge that is connected to the port, False otherwise.")
    .def("count_output_port_edges", &PyProcess::_count_output_port_edges
      , (arg("port"))
      , "The number of edges that are connected to the port.")
    .def("peek_at_port", &PyProcess::_peek_at_port
      , arg("port"), arg("idx") = 0
      , "Peek at a port.")
    .def("peek_at_datum_on_port", &PyProcess::_peek_at_datum_on_port
      , arg("port"), arg("idx") = 0
      , "Peek at a datum on a port.")
    .def("grab_from_port", &PyProcess::_grab_from_port
      , (arg("port"))
      , "Grab a datum packet from a port.")
    .def("grab_value_from_port", &PyProcess::_grab_value_from_port
      , (arg("port"))
      , "Grab a value from a port.")
    .def("grab_datum_from_port", &PyProcess::_grab_datum_from_port
      , (arg("port"))
      , "Grab a datum from a port.")
    .def("push_to_port", &PyProcess::_push_to_port
      , arg("port"), arg("datum")
      , "Push a datum packet to a port.")
    .def("push_value_to_port", &PyProcess::_push_value_to_port
      , arg("port"), arg("value")
      , "Push a value to a port.")
    .def("push_datum_to_port", &PyProcess::_push_datum_to_port
      , arg("port"), arg("datum")
      , "Push a datum to a port.")
    .def("get_config", &PyProcess::_get_config
      , "Gets the configuration for the process.")
    .def("config_value", &PyProcess::_config_value
      , (arg("key"))
      , "Gets a value from the configuration for the process.")
    .def("set_data_checking_level", &PyProcess::_set_data_checking_level
      , (arg("check"))
      , "Set the level to which the inputs are automatically checked.")
    .def("edge_data_info", &PyProcess::_edge_data_info
      , (arg("data"))
      , "Returns information about the given data.")
  ;

}

