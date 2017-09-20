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

#include <sprokit/python/util/python_exceptions.h>
#include <sprokit/python/util/python_gil.h>

#include <pybind11/pybind11.h>

/**
 * \file PyProcess.cxx
 *
 * \brief Python wrapper for \link sprokit::process\endlink.
 */

using namespace pybind11;

class PyProcess
  : public sprokit::process
{
  public:
    PyProcess(kwiver::vital::config_block_sptr const& config);
    ~PyProcess();


    void
    _configure() override
    {
      PYBIND11_OVERLOAD(
        void,
        sprokit::process,
        _configure
      );
    }

    void
    _init() override
    {
      PYBIND11_OVERLOAD(
        void,
        sprokit::process,
        _init
      );
    }

    void
    _reset() override
    {
      PYBIND11_OVERLOAD(
        void,
        sprokit::process,
        _reset
      );
    }

    void
    _flush() override
    {
      PYBIND11_OVERLOAD(
        void,
        sprokit::process,
        _flush
      );
    }

    void
    _step() override
    {
      PYBIND11_OVERLOAD(
        void,
        sprokit::process,
        _step
      );
    }

    void
    _reconfigure(kwiver::vital::config_block_sptr const& conf) override
    {
      PYBIND11_OVERLOAD(
        void,
        sprokit::process,
        _reconfigure,
        conf
      );
    }

    sprokit::process::properties_t
    _properties() const override
    {
      PYBIND11_OVERLOAD(
        sprokit::process::properties_t,
        sprokit::process,
        _properties
      );
    }

    sprokit::process::ports_t
    _input_ports() const override
    {
      PYBIND11_OVERLOAD(
        sprokit::process::ports_t,
        sprokit::process,
        _input_ports
      );
    }

    sprokit::process::ports_t
    _output_ports() const override
    {
      PYBIND11_OVERLOAD(
        sprokit::process::ports_t,
        sprokit::process,
        _output_ports
      );
    }

    sprokit::process::port_info_t
    _input_port_info(port_t const& port) override
    {
      PYBIND11_OVERLOAD(
        sprokit::process::port_info_t,
        sprokit::process,
        _input_port_info,
        port
      );
    }

    sprokit::process::port_info_t
    _output_port_info(port_t const& port) override
    {
      PYBIND11_OVERLOAD(
        sprokit::process::port_info_t,
        sprokit::process,
        _output_port_info,
        port
      );
    }

    bool
    _set_input_port_type(port_t const& port, port_type_t const& new_type) override
    {
      PYBIND11_OVERLOAD(
        bool,
        sprokit::process,
        _set_input_port_type,
        port,
        new_type
      );
    }

    bool
    _set_output_port_type(port_t const& port, port_type_t const& new_type) override
    {
      PYBIND11_OVERLOAD(
        bool,
        sprokit::process,
        _set_output_port_type,
        port,
        new_type
      );
    }

    kwiver::vital::config_block_keys_t
    _available_config() const override
    {
      PYBIND11_OVERLOAD(
        kwiver::vital::config_block_keys_t,
        sprokit::process,
        _available_config
      );
    }

    sprokit::process::conf_info_t
    _config_info(kwiver::vital::config_block_key_t const& key) override
    {
      PYBIND11_OVERLOAD(
        sprokit::process::conf_info_t,
        sprokit::process,
        _config_info,
        key
      );
    }

    void _declare_input_port(port_t const& port, port_info_t const& info);
    void _declare_input_port_1(port_t const& port,
                               port_type_t const& type_,
                               port_flags_t const& flags_,
                               port_description_t const& description_,
                               port_frequency_t const& frequency_);
    void _declare_output_port(port_t const& port, port_info_t const& info);
    void _declare_output_port_1(port_t const& port,
                                port_type_t const& type_,
                                port_flags_t const& flags_,
                                port_description_t const& description_,
                                port_frequency_t const& frequency_);

    void _set_input_port_frequency(port_t const& port, port_frequency_t const& new_frequency);
    void _set_output_port_frequency(port_t const& port, port_frequency_t const& new_frequency);

    void _remove_input_port(port_t const& port);
    void _remove_output_port(port_t const& port);

    void _declare_configuration_key(kwiver::vital::config_block_key_t const& key, conf_info_t const& info);
    void _declare_configuration_key_1(kwiver::vital::config_block_key_t const& key,
                                      kwiver::vital::config_block_value_t const& def_,
                                      kwiver::vital::config_block_description_t const& description_);
    void _declare_configuration_key_2(kwiver::vital::config_block_key_t const& key,
                                      kwiver::vital::config_block_value_t const& def_,
                                      kwiver::vital::config_block_description_t const& description_,
                                      bool tunable_);

    void _mark_process_as_complete();

    bool _has_input_port_edge(port_t const& port) const;
    size_t _count_output_port_edges(port_t const& port) const;

    sprokit::edge_datum_t _peek_at_port(port_t const& port, size_t idx) const;
    sprokit::datum_t _peek_at_datum_on_port(port_t const& port, size_t idx) const;
    sprokit::edge_datum_t _grab_from_port(port_t const& port) const;
    sprokit::datum_t _grab_datum_from_port(port_t const& port) const;
    object _grab_value_from_port(port_t const& port) const;
    void _push_to_port(port_t const& port, sprokit::edge_datum_t const& dat) const;
    void _push_datum_to_port(port_t const& port, sprokit::datum_t const& dat) const;
    void _push_value_to_port(port_t const& port, object const& obj) const;

    kwiver::vital::config_block_sptr _get_config() const;
    kwiver::vital::config_block_value_t _config_value(kwiver::vital::config_block_key_t const& key) const;

    void _set_data_checking_level(data_check_t check);

    data_info_t _edge_data_info(sprokit::edge_data_t const& data);
};

PyProcess
::PyProcess(kwiver::vital::config_block_sptr const& config)
  : sprokit::process(config)
{
}

PyProcess
::~PyProcess()
{
}

void
PyProcess
::_declare_input_port(port_t const& port, port_info_t const& info)
{
  declare_input_port(port, info);
}

void
PyProcess
::_declare_input_port_1(port_t const& port,
                        port_type_t const& type_,
                        port_flags_t const& flags_,
                        port_description_t const& description_,
                        port_frequency_t const& frequency_)
{
  declare_input_port(port, type_, flags_, description_, frequency_);
}

void
PyProcess
::_declare_output_port(port_t const& port, port_info_t const& info)
{
  declare_output_port(port, info);
}

void
PyProcess
::_declare_output_port_1(port_t const& port,
                         port_type_t const& type_,
                         port_flags_t const& flags_,
                         port_description_t const& description_,
                         port_frequency_t const& frequency_)
{
  declare_output_port(port, type_, flags_, description_, frequency_);
}

void
PyProcess
::_set_input_port_frequency(port_t const& port, port_frequency_t const& new_frequency)
{
  set_input_port_frequency(port, new_frequency);
}

void
PyProcess
::_set_output_port_frequency(port_t const& port, port_frequency_t const& new_frequency)
{
  set_output_port_frequency(port, new_frequency);
}

void
PyProcess
::_remove_input_port(port_t const& port)
{
  remove_input_port(port);
}

void
PyProcess
::_remove_output_port(port_t const& port)
{
  remove_output_port(port);
}

void
PyProcess
::_declare_configuration_key(kwiver::vital::config_block_key_t const& key, conf_info_t const& info)
{
  declare_configuration_key(key, info);
}

void
PyProcess
::_declare_configuration_key_1(kwiver::vital::config_block_key_t const& key,
                               kwiver::vital::config_block_value_t const& def_,
                               kwiver::vital::config_block_description_t const& description_)
{
  declare_configuration_key(key, def_, description_);
}

void
PyProcess
::_declare_configuration_key_2(kwiver::vital::config_block_key_t const& key,
                               kwiver::vital::config_block_value_t const& def_,
                               kwiver::vital::config_block_description_t const& description_,
                               bool tunable_)
{
  declare_configuration_key(key, def_, description_, tunable_);
}

void
PyProcess
::_mark_process_as_complete()
{
  mark_process_as_complete();
}

bool
PyProcess
::_has_input_port_edge(port_t const& port) const
{
  return has_input_port_edge(port);
}

size_t
PyProcess
::_count_output_port_edges(port_t const& port) const
{
  return count_output_port_edges(port);
}

sprokit::edge_datum_t
PyProcess
::_peek_at_port(port_t const& port, size_t idx) const
{
  return peek_at_port(port, idx);
}

sprokit::datum_t
PyProcess
::_peek_at_datum_on_port(port_t const& port, size_t idx) const
{
  return peek_at_datum_on_port(port, idx);
}

sprokit::edge_datum_t
PyProcess
::_grab_from_port(port_t const& port) const
{
  return grab_from_port(port);
}

sprokit::datum_t
PyProcess
::_grab_datum_from_port(port_t const& port) const
{
  return grab_datum_from_port(port);
}

object
PyProcess
::_grab_value_from_port(port_t const& port) const
{
  sprokit::python::python_gil const gil;

  (void)gil;

  sprokit::datum_t const dat = grab_datum_from_port(port);
  boost::any const any = dat->get_datum<boost::any>();

  return cast(any);
}

void
PyProcess
::_push_to_port(port_t const& port, sprokit::edge_datum_t const& dat) const
{
  return push_to_port(port, dat);
}

void
PyProcess
::_push_datum_to_port(port_t const& port, sprokit::datum_t const& dat) const
{
  return push_datum_to_port(port, dat);
}

void
PyProcess
::_push_value_to_port(port_t const& port, object const& obj) const
{
  sprokit::python::python_gil const gil;

  (void)gil;

  boost::any const any = obj.cast< boost::any >();
  sprokit::datum_t const dat = sprokit::datum::new_datum(any);

  return push_datum_to_port(port, dat);
}

kwiver::vital::config_block_sptr
PyProcess
::_get_config() const
{
  return get_config();
}

kwiver::vital::config_block_value_t
PyProcess
::_config_value(kwiver::vital::config_block_key_t const& key) const
{
  return config_value<kwiver::vital::config_block_value_t>(key);
}

void
PyProcess
::_set_data_checking_level(data_check_t check)
{
  set_data_checking_level(check);
}

sprokit::process::data_info_t
PyProcess
::_edge_data_info(sprokit::edge_data_t const& data)
{
  return edge_data_info(data);
}
