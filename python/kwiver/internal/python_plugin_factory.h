// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef PYTHON_PLUGIN_FACTORY_H
#define PYTHON_PLUGIN_FACTORY_H

#include <pybind11/pybind11.h>

#include <vital/plugin_management/plugin_factory.h>

namespace py = pybind11;

namespace kwiver::vital::python {

/// @brief Factory to register and generate python instances for an interface.
class python_plugin_factory
  : public plugin_factory
{
public:
  explicit python_plugin_factory( py::object const& python_type )
    : m_python_type( python_type )
  {
    this->add_attribute( plugin_factory::INTERFACE_TYPE,
                         python_type.attr( "interface_name" )()
                           .cast< std::string > () )
                         .add_attribute( plugin_factory::CONCRETE_TYPE,
                                         python_type.attr( "__name__" ).cast<
                                             std::string > () )
                           .add_attribute( plugin_factory::PLUGIN_NAME,
                                           python_type.attr( "__name__" ).cast<
                                               std::string > () );
  }

  ~python_plugin_factory() override = default;

  pluggable_sptr
  from_config( const config_block_sptr cb ) const override
  {
    py::object instance = m_python_type.attr( "from_config" )( cb );
    return instance.cast< pluggable_sptr >();
  }

  void
  get_default_config( config_block& cb ) const override
  {
    m_python_type.attr( "get_default_config" )( cb );
  }

private:
  py::object m_python_type;
};

} // namespace

#endif // PYTHON_PLUGIN_FACTORY_H
