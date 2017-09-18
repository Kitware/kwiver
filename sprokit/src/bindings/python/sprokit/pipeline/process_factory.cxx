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

/**
 * \file process_factory.cxx
 *
 * \brief Python bindings for \link sprokit::process_factory\endlink.
 */

#include <sprokit/pipeline/process.h>
#include <sprokit/pipeline/process_cluster.h>
#include <sprokit/pipeline/process_factory.h>

#include <sprokit/python/util/python_exceptions.h>
#include <sprokit/python/util/python_gil.h>
#include <sprokit/python/util/python_threading.h>

#include <vital/plugin_loader/plugin_manager.h>
#include <vital/vital_foreach.h>

#include <pybind11/pybind11.h>

#ifdef WIN32
 // Windows get_pointer const volatile workaround
namespace boost
{
  template <> inline sprokit::process const volatile*
  get_pointer(class sprokit::process const volatile* p)
  {
    return p;
  }
  template <> inline sprokit::process_cluster const volatile*
  get_pointer(class sprokit::process_cluster const volatile* p)
  {
    return p;
  }
}
#endif

using namespace pybind11;

static void register_process( sprokit::process::type_t const& type,
                              sprokit::process::description_t const& desc,
                              object obj );

static bool is_process_loaded( const std::string& name );
static void mark_process_loaded( const std::string& name );
static std::string get_description( const std::string& name );
static std::vector< std::string > process_names();

// ==================================================================
PYBIND11_MODULE(process_factory, m)
{

  class_<sprokit::process::description_t>(m, "ProcessDescription"
    , "The type for a description of a process type.");
  class_<kwiver::vital::plugin_manager::module_t>(m, "ProcessModule"
    , "The type for a process module name.");

  class_<sprokit::process_t>(m, "Process"
    , "The base class of processes.")
    .def("configure", &sprokit::process::configure
      , "Configures the process.")
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
      , arg("port")
      , "Returns information about the given input port.")
    .def("output_port_info", &sprokit::process::output_port_info
      , arg("port")
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
    .def_readonly_static("flag_input_static", &sprokit::process::flag_input_static)
    .def_readonly_static("flag_input_mutable", &sprokit::process::flag_input_mutable)
    .def_readonly_static("flag_input_nodep", &sprokit::process::flag_input_nodep)
    .def_readonly_static("flag_required", &sprokit::process::flag_required)
  ;

  class_<sprokit::processes_t>(m, "Processes"
    , "A collection of processes.")
  ;

  class_<sprokit::process_cluster_t>
    (m, "ProcessCluster", "The base class of process clusters.");

  m.def("is_process_module_loaded", &is_process_loaded
      , (arg("module"))
      , "Returns True if the module has already been loaded, False otherwise.");

  m.def("mark_process_module_as_loaded", &mark_process_loaded
      , (arg("module"))
      , "Marks a module as loaded.");

  m.def("add_process", &register_process
      , arg("type"), arg("description"), arg("ctor")
      , "Registers a function which creates a process of the given type.");

  m.def("create_process", &sprokit::create_process
      , arg("type"), arg("name"), arg("config") = kwiver::vital::config_block::empty_config()
      , "Creates a new process of the given type.");

  m.def("description", &get_description
      , (arg("type"))
      , "Returns description for the process");

  m.def("types", &process_names
      , "Returns list of process names" );

}


// ==================================================================
class python_process_wrapper
  : sprokit::python::python_threading
{
public:
  python_process_wrapper( object obj );
  ~python_process_wrapper();

  sprokit::process_t operator()( kwiver::vital::config_block_sptr const& config );


private:
  object const m_obj;
};


// ------------------------------------------------------------------
void
register_process( sprokit::process::type_t const&        type,
                  sprokit::process::description_t const& desc,
                  object                                 obj )
{
  sprokit::python::python_gil const gil;

  (void)gil;

  python_process_wrapper const wrap( obj );

  kwiver::vital::plugin_manager& vpm = kwiver::vital::plugin_manager::instance();
  sprokit::process::type_t derived_type = "python::";
  auto fact = vpm.add_factory( new sprokit::process_factory( derived_type + type, // derived type name string
                                                             typeid( sprokit::process ).name(),
                                                             wrap ) );

  fact->add_attribute( kwiver::vital::plugin_factory::PLUGIN_NAME, type )
    .add_attribute( kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME, "python-runtime" )
    .add_attribute( kwiver::vital::plugin_factory::PLUGIN_DESCRIPTION, desc )
    ;
}


// ------------------------------------------------------------------
bool is_process_loaded( const std::string& name )
{
  kwiver::vital::plugin_manager& vpm = kwiver::vital::plugin_manager::instance();
  return vpm.is_module_loaded( name );
}


// ------------------------------------------------------------------
void mark_process_loaded( const std::string& name )
{
  kwiver::vital::plugin_manager& vpm = kwiver::vital::plugin_manager::instance();
  vpm.mark_module_as_loaded( name );
}


// ------------------------------------------------------------------
std::string get_description( const std::string& type )
{
  typedef kwiver::vital::implementation_factory_by_name< sprokit::process > proc_factory;
  proc_factory ifact;

  kwiver::vital::plugin_factory_handle_t a_fact;
  SPROKIT_PYTHON_TRANSLATE_EXCEPTION(
    a_fact = ifact.find_factory( type );
    )

  std::string buf = "-- Not Set --";
  a_fact->get_attribute( kwiver::vital::plugin_factory::PLUGIN_DESCRIPTION, buf );

  return buf;
}


// ------------------------------------------------------------------
std::vector< std::string > process_names()
{
  kwiver::vital::plugin_manager& vpm = kwiver::vital::plugin_manager::instance();
  auto fact_list = vpm.get_factories<sprokit::process>();

  std::vector<std::string> name_list;
  VITAL_FOREACH( auto fact, fact_list )
  {
    std::string buf;
    if (fact->get_attribute( kwiver::vital::plugin_factory::PLUGIN_NAME, buf ))
    {
      name_list.push_back( buf );
    }
  } // end foreach

  return name_list;
}

// ------------------------------------------------------------------
python_process_wrapper
  ::python_process_wrapper( object obj )
  : m_obj( obj )
{
}


python_process_wrapper
  ::~python_process_wrapper()
{
}


sprokit::process_t
python_process_wrapper
  ::operator()( kwiver::vital::config_block_sptr const& config )
{
  sprokit::python::python_gil const gil;

  (void)gil;

  object proc;

  SPROKIT_PYTHON_HANDLE_EXCEPTION( proc = m_obj( config ) )

  return proc.cast< sprokit::process_t >();
}
