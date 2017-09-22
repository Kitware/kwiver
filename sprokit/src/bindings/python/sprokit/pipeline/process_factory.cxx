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
#include <iostream>
#include <sprokit/pipeline/process.h>
#include <sprokit/pipeline/process_cluster.h>
#include <sprokit/pipeline/process_factory.h>

#include <sprokit/python/util/python_exceptions.h>
#include <sprokit/python/util/python_gil.h>
#include <sprokit/python/util/python_threading.h>

#include <vital/plugin_loader/plugin_manager.h>
#include <vital/vital_foreach.h>

#include <pybind11/pybind11.h>
#include "PyProcess.cxx"

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
static PyProcess create_process( sprokit::process::type_t& type,
                                 sprokit::process::name_t& name,
                                 kwiver::vital::config_block_sptr config = kwiver::vital::config_block::empty_config() ); 
// ==================================================================
PYBIND11_MODULE(process_factory, m)
{
  class_<sprokit::processes_t>(m, "Processes"
    , "A collection of processes.");

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

  m.def("create_process", &create_process
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

//-------------------------------------------------------------------
PyProcess
create_process(sprokit::process::type_t& type,
               sprokit::process::name_t& name,
               kwiver::vital::config_block_sptr config)
{
  sprokit::process_t process = sprokit::create_process(type, name, config);
  return PyProcess_from_process(process); 
}

// -------------------------------------------------------------------
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
