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

#include <sprokit/pipeline/edge.h>
#include <sprokit/pipeline/process.h>
#include <sprokit/pipeline/stamp.h>
#include <sprokit/pipeline/process_cluster.h>

#include <sprokit/python/util/python_exceptions.h>
#include <sprokit/python/util/python_gil.h>

#include <pybind11/pybind11.h>

/**
 * \file PyProcess.cxx
 *
 * \brief Python wrapper for \link sprokit::process\endlink.
 */

using namespace pybind11;

// Instead of just using a sprokit::process, we need a wrapper
// This is because of how pybind11 deals with shared_ptrs 

class PubProcess : public sprokit::process
{
  public:

  // Because sprokit::process is noncopyable
  // We need to manually create a copy constructor to use shared_ptrs properly 
  PubProcess(kwiver::vital::config_block_sptr const& config) : sprokit::process(config) {};
  PubProcess(PubProcess const& other) : sprokit::process(other.get_config()) {d = other.d;}; 
};

class PyProcess
{
  public:
  PyProcess(std::shared_ptr<PubProcess> ptr) : process_ptr(ptr) {}
  PyProcess(kwiver::vital::config_block_sptr const& config);

  std::shared_ptr<PubProcess> process_ptr;
/*
  void declare_input_port(sprokit::process::port_t const&, sprokit::process::port_info_t const&);
  void declare_input_port(sprokit::process::port_t const&, sprokit::process::port_type_t const&,
                          sprokit::process::port_flags_t const&, sprokit::process::port_description_t const&,
                          sprokit::process::port_frequency_t const&);

  void declare_output_port(sprokit::process::port_t const&, sprokit::process::port_info_t const&);
  void declare_output_port(sprokit::process::port_t const&, sprokit::process::port_info_t const&,
                           sprokit::process::port_flags_t const&, sprokit::process::port_description_t const&,
                           sprokit::process::port_frequency_t const&);

  void set_input_port_frequency(sprokit::process::port_t const&, sprokit::process::port_frequency_t const&);
  void set_output_port_frequency(sprokit::process::port_t const&, sprokit::process::port_frequency_t const&);

  void remove_input_port(sprokit::process::port_t const&);
  void remove_output_port(sprokit::process::port_t const&);


  void declare_configuration_key(kwiver::vital::config_block_key_t const&,
                                 sprokit::process::conf_info_t const&);
  void declare_configuration_key(kwiver::vital::config_block_key_t const&,
                                 kwiver::vital::config_block_value_t const&,
                                 kwiver::vital::config_block_description_t const&);
  void declare_configuration_key(kwiver::vital::config_block_key_t const&,
                                 kwiver::vital::config_block_value_t const&,
                                 kwiver::vital::config_block_description_t const&,
                                 bool);

  void mark_process_as_complete();

  bool has_input_port_edge(sprokit::process::port_t const&);
  size_t count_output_port_edges(sprokit::process::port_t const&);

  sprokit::edge_datum_t peek_at_port(sprokit::process::port_t const&, size_t);
  sprokit::datum_t peek_at_datum_on_port (sprokit::process::port_t const&, size_t);
  sprokit::edge_datum_t grab_from_port(sprokit::process::port_t const&);
  sprokit::datum_t grab_datum_from_port(sprokit::process::port_t const&);
  object grab_value_from_port(sprokit::process::port_t const&);
  void push_to_port(sprokit::process::port_t const&, sprokit::edge_datum_t const&);
  void push_datum_to_port(sprokit::process::port_t const&, sprokit::datum_t const&);
  void push_value_to_port(sprokit::process::port_t const&, object const&);

  kwiver::vital::config_block_sptr get_config() const;
  kwiver::vital::config_block_value_t config_value(kwiver::vital::config_block_key_t const&) const;

  void set_data_checking_level(sprokit::process::data_check_t);

  sprokit::process::data_info_t edge_data_info(sprokit::edge_data_t const&); 
*/
};

PyProcess
::PyProcess(kwiver::vital::config_block_sptr const& config)
{
  process_ptr = std::make_shared<PubProcess>(PubProcess(config));
}

/*
void
PyProcess
::declare_input_port(sprokit::process::port_t const& port, sprokit::process::port_info_t const& info)
{
  process_ptr->declare_input_port(port, info);
}

void
PyProcess
::declare_input_port(sprokit::process::port_t const& port,
                        sprokit::process::port_type_t const& type_,
                        sprokit::process::port_flags_t const& flags_,
                        sprokit::process::port_description_t const& description_,
                        sprokit::process::port_frequency_t const& frequency_)
{
  process_ptr->declare_input_port(port, type_, flags_, description_, frequency_);
}

void
PyProcess
::declare_output_port(sprokit::process::port_t const& port, sprokit::process::port_info_t const& info)
{
  process_ptr->declare_output_port(port, info);
}

void
PyProcess
::declare_output_port(sprokit::process::port_t const& port,
                         sprokit::process::port_type_t const& type_,
                         sprokit::process::port_flags_t const& flags_,
                         sprokit::process::port_description_t const& description_,
                         sprokit::process::port_frequency_t const& frequency_)
{
  process_ptr->declare_output_port(port, type_, flags_, description_, frequency_);
}

void
PyProcess
::set_input_port_frequency(sprokit::process::port_t const& port, sprokit::process::port_frequency_t const& new_frequency)
{
  process_ptr->set_input_port_frequency(port, new_frequency);
}

void
PyProcess
::set_output_port_frequency(sprokit::process::port_t const& port, sprokit::process::port_frequency_t const& new_frequency)
{
  process_ptr->set_output_port_frequency(port, new_frequency);
}

void
PyProcess
::remove_input_port(sprokit::process::port_t const& port)
{
  process_ptr->remove_input_port(port);
}

void
PyProcess
::remove_output_port(sprokit::process::port_t const& port)
{
  process_ptr->remove_output_port(port);
}

void
PyProcess
::declare_configuration_key(kwiver::vital::config_block_key_t const& key,
                            sprokit::process::conf_info_t const& info)
{
  process_ptr->declare_configuration_key(key, info);
}

void
PyProcess
::declare_configuration_key(kwiver::vital::config_block_key_t const& key,
                               kwiver::vital::config_block_value_t const& def_,
                               kwiver::vital::config_block_description_t const& description_)
{
  process_ptr->declare_configuration_key(key, def_, description_);
}

void
PyProcess
::declare_configuration_key(kwiver::vital::config_block_key_t const& key,
                               kwiver::vital::config_block_value_t const& def_,
                               kwiver::vital::config_block_description_t const& description_,
                               bool tunable_)
{
  process_ptr->declare_configuration_key(key, def_, description_, tunable_);
}

void
PyProcess
::mark_process_as_complete()
{
  process_ptr->mark_process_as_complete();
}

bool
PyProcess
::has_input_port_edge(sprokit::process::port_t const& port) const
{
  return process_ptr->has_input_port_edge(port);
}

size_t
PyProcess
::count_output_port_edges(sprokit::process::port_t const& port) const
{
  return process_ptr->count_output_port_edges(port);
}

sprokit::edge_datum_t
PyProcess
::peek_at_port(sprokit::process::port_t const& port, size_t idx) const
{
  return process_ptr->peek_at_port(port, idx);
}

sprokit::datum_t
PyProcess
::peek_at_datum_on_port(sprokit::process::port_t const& port, size_t idx) const
{
  return process_ptr->peek_at_datum_on_port(port, idx);
}

sprokit::edge_datum_t
PyProcess
::grab_from_port(sprokit::process::port_t const& port) const
{
  return process_ptr->grab_from_port(port);
}

sprokit::datum_t
PyProcess
::grab_datum_from_port(sprokit::process::port_t const& port) const
{
  return process_ptr->grab_datum_from_port(port);
}

object
PyProcess
::grab_value_from_port(sprokit::process::port_t const& port) const
{
  sprokit::python::python_gil const gil;

  (void)gil;

  sprokit::datum_t const dat = grab_datum_from_port(port);
  boost::any const any = dat->get_datum<boost::any>();

  return cast(any);
}

void
PyProcess
::push_to_port(sprokit::process::port_t const& port, sprokit::edge_datum_t const& dat) const
{
  return process_ptr->push_to_port(port, dat);
}

void
PyProcess
::push_datum_to_port(sprokit::process::port_t const& port, sprokit::datum_t const& dat) const
{
  return process_ptr->push_datum_to_port(port, dat);
}

void
PyProcess
::push_value_to_port(sprokit::process::port_t const& port, object const& obj) const
{
  sprokit::python::python_gil const gil;

  (void)gil;

  boost::any const any = obj.cast< boost::any >();
  sprokit::datum_t const dat = sprokit::datum::new_datum(any);

  return push_datum_to_port(port, dat);
}

kwiver::vital::config_block_sptr
PyProcess
::get_config() const
{
  return process_ptr->get_config();
}

kwiver::vital::config_block_value_t
PyProcess
::config_value(kwiver::vital::config_block_key_t const& key) const
{
  return process_ptr->config_value<kwiver::vital::config_block_value_t>(key);
}

void
PyProcess
::set_data_checking_level(sprokit::process::data_check_t check)
{
  process_ptr->set_data_checking_level(check);
}

sprokit::process::data_info_t
PyProcess
::edge_data_info(sprokit::edge_data_t const& data)
{
  return process_ptr->edge_data_info(data);
}
*/

PyProcess
PyProcess_from_process(sprokit::process_t const& process)
{
  PubProcess* process_pub = (PubProcess*) (process.get());
  return PyProcess(std::make_shared<PubProcess> (*process_pub));
}
