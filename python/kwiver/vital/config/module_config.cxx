// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <vital/config/config_block.h>
#include <vital/config/config_difference.h>
#include <vital/types/geo_polygon.h>
#include <vital/util/wrap_text_block.h>

#include "module_config_helpers.h"

namespace py = pybind11;
namespace kv = kwiver::vital;

PYBIND11_MODULE( _config, m )
{
  m.doc() =
    R"pbdoc(
Config module for vital
-----------------------

.. currentmodule:: config

.. autosummary::
   :toctree: _generate

empty_config
ConfigKeys
Config
)pbdoc";

  // -------------------------------------------------------------------------
  m.def( "empty_config", &kv::config_block::empty_config,
         py::arg( "name" ) = kv::config_block_key_t(),
         // doc-string
         "Returns an empty :class:`kwiver.vital.config.Config` object"
         );

  // -------------------------------------------------------------------------
  // TODO: Why is this a thing? What is it for?
  //       If we cannot answer this, remove it.
  py::bind_vector< std::vector< std::string > >(
    m, "ConfigKeys",
    // doc-string
    "A collection of keys for a configuration."
    );

  // -------------------------------------------------------------------------
  py::class_< kv::config_block, kv::config_block_sptr >(
    m, "Config",
    // doc-string
    "A key-value store of configuration values."
    )
    .def( "subblock", &kv::config_block::subblock,
          py::arg( "name" ),
          py::doc(
            "Returns a :class:`kwiver.vital.config.Config` from the "
            "configuration using the name of the subblock.\n"
            "\n"
            "The object is a copy of the block in the configuration.\n"
            "\n"
            ":param name: The name of the subblock in a "
            ":class:`kwiver.vital.config.Config` object.\n"
            ":return: a subblock of type :class:`kwiver.vital.config.Config`"
            )
          )
    .def( "subblock_view", &kv::config_block::subblock_view,
          py::arg( "name" ),
          py::doc(
            "Returns a :class:`kwiver.vital.config.Config` from the "
            "configuration using the name of the subblock.\n"
            "\n"
            "The object is a view rather than the copy of the block in the "
            "configuration.\n"
            ":param name: The name of the subblock in a :class:`kwiver.vital."
            "config.Config` object\n"
            ":return: a subblock of type :class:`kwiver.vital.config.Config`"
            )
          )
    .def( "get_value",
          ( kv::config_block_value_t
            ( kv::config_block::* )( kv::config_block_key_t const& ) const ) &
          kv::config_block::get_value< std::string >,
          py::arg( "key" ),
          py::doc(
            "Retrieve a value from the configuration using key.\n"
            "\n"
            ":param key: key in the configuration\n"
            ":return: A string value associated with the key"
            )
          )
    .def( "get_value",
          ( kv::config_block_value_t
            ( kv::config_block::* )( kv::config_block_key_t const&,
                                     kv::config_block_value_t const& ) const
            noexcept ) &
          kv::config_block::get_value,
          py::arg( "key" ), py::arg( "default" ),
          py::doc(
            "Retrieve a value from the configuration, using a default in case "
            "of failure.\n"
            "\n"
            ":param key: A key in the configuration\n"
            ":param default: A default value for the key\n"
            ":return: A string value associated with the key"
            )
          )
    .def( "get_value_geo_poly",
          ( kv::geo_polygon
            ( kv::config_block::* )( kv::config_block_key_t const& ) const ) &
          kv::config_block::get_value< kv::geo_polygon >,
          py::arg( "key" ),
          py::doc(
            "Retrieve a geo_polygon value from the configuration using key\n"
            "\n"
            ":param key: key in the configuration\n"
            ":return: A string value associated with the key"
            )
          )
    .def( "get_value_geo_poly",
          ( kv::geo_polygon
            ( kv::config_block::* )( kv::config_block_key_t const&,
                                     kv::geo_polygon const& ) const ) &
          kv::config_block::get_value< kv::geo_polygon >,
          py::arg( "key" ), py::arg( "default" ),
          py::doc(
            "Retrieve a geo_polygon value from the configuration using key, "
            "using a default in case of failure.\n"
            "\n"
            ":param key: key in the configuration\n"
            ":param default: A default geo_polygon value for the key\n"
            ":return: A string value associated with the key"
            )
          )
    .def( "set_value",
          ( void ( kv::config_block::* )( kv::config_block_key_t const&,
                                          kv::config_block_value_t const& ) ) &
          kv::config_block::set_value< kv::config_block_value_t >,
          py::arg( "key" ), py::arg( "value" ),
          py::doc(
            "Set a value in the configuration.\n"
            "\n"
            ":param key: A key in the configuration.\n"
            ":param value: A value in the configuration.\n"
            ":return: None"
            )
          )
    .def( "set_value_geo_poly",
          ( void ( kv::config_block::* )( kv::config_block_key_t const&,
                                          kv::geo_polygon const& ) ) &
          kv::config_block::set_value< kv::geo_polygon >,
          py::arg( "key" ), py::arg( "value" ),
          py::doc(
            "Set a value in the configuration using "
            "config_block_set_value_cast<geo_polygon>\n"
            "\n"
            ":param key: A key in the configuration.\n"
            ":param value: A value in the configuration.\n"
            ":return: None"
            )
          )
    .def( "unset_value", &kv::config_block::unset_value,
          py::arg( "key" ),
          py::doc(
            "Unset a value in the configuration.\n"
            "\n"
            ":param key: A key in the configuration\n"
            ":return: None"
            )
          )
    .def( "is_read_only", &kv::config_block::is_read_only,
          py::arg( "key" ),
          py::doc(
            "Check if a key is marked as read only.\n"
            "\n"
            ":param key: A key in the configuration\n"
            ":return: Boolean specifying if the key value pair is read only"
            )
          )
    .def( "mark_read_only", &kv::config_block::mark_read_only,
          py::arg( "key" ),
          py::doc(
            "Mark a key as read only.\n"
            "\n"
            ":param key: A key in the configuration.\n"
            ":return: None"
            )
          )
    .def( "merge_config", &kv::config_block::merge_config,
          py::arg( "config" ),
          py::doc(
            "Merge another configuration block into the current one.\n"
            "\n"
            ":param config: An object of :class:`vital.config.Config`\n"
            ":return: An object of :class:`vital.config.Config` containing "
            "the merged configuration"
            )
          )
    .def( "available_values", &kv::config_block::available_values,
          py::doc(
            "Retrieves the list of available values in the configuration.\n"
            "\n"
            ":return: A list of string with all the keys"
            )
          )
    .def( "has_value", &kv::config_block::has_value,
          py::arg( "key" ),
          py::doc(
            "Returns True if the key is set.\n"
            "\n"
            ":param key: A key in the configuration\n"
            ":return: Boolean specifying if the key is present in the "
            "configuration"
            )
          )
    .def_static( "block_sep", &kv::config_block::block_sep,
                 "The string which separates block names from key names." )
    .def_static( "global_value", &kv::config_block::global_value,
                 "A special key which is automatically inherited on subblock "
                 "requests." )
    .def( "__len__", &kv::python::config_len,
          "Magic function that return the length of the configuration block" )
    .def( "__contains__", &kv::config_block::has_value,
          "Magic function to check if an key is in the configuration" )
    .def( "__getitem__", &kv::python::config_getitem,
          "Magic function to get a value" )
    .def( "__setitem__", &kv::python::config_setitem,
          "Magic function to assign a new value to a key" )
    .def( "__delitem__", &kv::python::config_delitem,
          "Magic function to remove a key" )
  ;

  // -------------------------------------------------------------------------
  py::class_< kv::config_difference,
              std::shared_ptr< kv::config_difference > >(
    m, "ConfigDifference",
    "Represents difference between two config blocks"
    )
    .def( py::init< kv::config_block_sptr, kv::config_block_sptr >(),
          py::doc( "Determine difference between config blocks" ) )
    .def( py::init< kv::config_block_keys_t, kv::config_block_sptr >(),
          py::doc( "Determine difference between config blocks" ) )
    .def( "extra_keys", &kv::config_difference::extra_keys,
          "Return list of config keys that are not in the ref config" )
    .def( "unspecified_keys", &kv::config_difference::unspecified_keys,
          "Return list of config keys that are in reference config but not in "
          "the other config" )
  ;
}
