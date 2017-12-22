/*ckwg +29
 * Copyright 2011-2015 by Kitware, Inc.
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

#include <vital/config/config_block.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <sstream>

/**
 * \file config.cxx
 *
 * \brief Python bindings for \link kwiver::vital::config \endlink.
 */

namespace kwiver {
namespace vital {

// config value converter for pybind11
template<>
config_block_value_t
config_block_set_value_cast( pybind11::object const& value )
{
  return value.cast<std::string>();
}

} }


using namespace pybind11;

static void config_set_value( kwiver::vital::config_block_sptr         self,
                              kwiver::vital::config_block_key_t const& key,
                              kwiver::vital::config_block_key_t const& value  );

static kwiver::vital::config_block_value_t config_get_value( kwiver::vital::config_block_sptr         self,
                                                             kwiver::vital::config_block_key_t const& key );
static kwiver::vital::config_block_value_t config_get_value_with_default( kwiver::vital::config_block_sptr            self,
                                                                          kwiver::vital::config_block_key_t const&    key,
                                                                          kwiver::vital::config_block_value_t const&  def );
static pybind11::size_t config_len( kwiver::vital::config_block_sptr self );
static kwiver::vital::config_block_value_t config_getitem( kwiver::vital::config_block_sptr         self,
                                                           kwiver::vital::config_block_key_t const& key );
static void config_setitem( kwiver::vital::config_block_sptr          self,
                            kwiver::vital::config_block_key_t const&  key,
                            object const&                             value );
static void config_delitem( kwiver::vital::config_block_sptr          self,
                            kwiver::vital::config_block_key_t const&  key );


PYBIND11_MODULE(config, m)
{
  m.def("empty_config", &kwiver::vital::config_block::empty_config
    , arg("name") = kwiver::vital::config_block_key_t()
    , "Returns an empty configuration.");

  bind_vector<std::vector<std::string> >(m, "ConfigKeys"
    , "A collection of keys for a configuration.");

  class_<kwiver::vital::config_block, kwiver::vital::config_block_sptr>(m, "Config"
    , "A key-value store of configuration values")
    .def("subblock", &kwiver::vital::config_block::subblock
      , arg("name")
      , "Returns a subblock from the configuration.")
    .def("subblock_view", &kwiver::vital::config_block::subblock_view
      , arg("name")
      , "Returns a linked subblock from the configuration.")
    .def("get_value", &config_get_value
      , arg("key")
      , "Retrieve a value from the configuration.")
    .def("get_value", &config_get_value_with_default
      , arg("key"), arg("default")
      , "Retrieve a value from the configuration, using a default in case of failure.")
    .def("set_value", &config_set_value
      , arg("key"), arg("value")
      , "Set a value in the configuration.")
    .def("unset_value", &kwiver::vital::config_block::unset_value
      , arg("key")
      , "Unset a value in the configuration.")
    .def("is_read_only", &kwiver::vital::config_block::is_read_only
      , arg("key")
      , "Check if a key is marked as read only.")
    .def("mark_read_only", &kwiver::vital::config_block::mark_read_only
      , arg("key")
      , "Mark a key as read only.")
    .def("merge_config", &kwiver::vital::config_block::merge_config
      , arg("config")
      , "Merge another configuration block into the current one.")
    .def("available_values", &kwiver::vital::config_block::available_values
      , "Retrieves the list of available values in the configuration.")
    .def("has_value", &kwiver::vital::config_block::has_value
      , arg("key")
      , "Returns True if the key is set.")
    .def_readonly_static("block_sep", &kwiver::vital::config_block::block_sep
      , "The string which separates block names from key names.")
    .def_readonly_static("global_value", &kwiver::vital::config_block::global_value
      , "A special key which is automatically inherited on subblock requests.")
    .def("__len__", &config_len)
    .def("__contains__", &kwiver::vital::config_block::has_value)
    .def("__getitem__", &config_getitem)
    .def("__setitem__", &config_setitem)
    .def("__delitem__", &config_delitem)
  ;
}


void
config_set_value( kwiver::vital::config_block_sptr          self,
                  kwiver::vital::config_block_key_t const&  key,
                  kwiver::vital::config_block_key_t const&  value )
{
  return self->set_value< kwiver::vital::config_block_value_t > ( key, value );
}


kwiver::vital::config_block_value_t
config_get_value( kwiver::vital::config_block_sptr          self,
                  kwiver::vital::config_block_key_t const&  key )
{
  return self->get_value< kwiver::vital::config_block_value_t > ( key );
}


kwiver::vital::config_block_value_t
config_get_value_with_default( kwiver::vital::config_block_sptr           self,
                               kwiver::vital::config_block_key_t const&   key,
                               kwiver::vital::config_block_value_t const& def )
{
  return self->get_value< kwiver::vital::config_block_value_t > ( key, def );
}


pybind11::size_t
config_len( kwiver::vital::config_block_sptr self )
{
  return self->available_values().size();
}


kwiver::vital::config_block_value_t
config_getitem( kwiver::vital::config_block_sptr          self,
                kwiver::vital::config_block_key_t const&  key )
{
  kwiver::vital::config_block_value_t val;

  try
  {
    val = config_get_value( self, key );
  }
  catch ( kwiver::vital::no_such_configuration_value_exception const& )
  {
    std::ostringstream sstr;

    sstr << "\'" << key << "\'";

    PyErr_SetString( PyExc_KeyError, sstr.str().c_str() );
    throw error_already_set();
  }

  return val;
}


void
config_setitem( kwiver::vital::config_block_sptr          self,
                kwiver::vital::config_block_key_t const&  key,
                object const&                             value )
{
  kwiver::vital::config_block_key_t const& str_value = str(value);

  self->set_value( key, str_value );
}


void
config_delitem( kwiver::vital::config_block_sptr          self,
                kwiver::vital::config_block_key_t const&  key )
{
  try
  {
    self->unset_value( key );
  }
  catch ( kwiver::vital::no_such_configuration_value_exception const& )
  {
    std::ostringstream sstr;

    sstr << "\'" << key << "\'";

    PyErr_SetString( PyExc_KeyError, sstr.str().c_str() );
    throw error_already_set();
  }
}
