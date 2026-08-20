// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef CONFIG_OPTIONS_HELPERS_TXX
#define CONFIG_OPTIONS_HELPERS_TXX

#include <vital/config/config_helpers.txx>

#include <arrows/ceres/options.h>
#include <arrows/ceres/types.h>

namespace kwiver::vital {

// ----------------------------------------------------------------------------
// specializations of set/get_config_helper for solver_options
// for base implementations see config_helpers.txx

/// A helper for populating \p key in \p config based on the configuration of
/// the solver_options given in \p value.
template < typename ValueType,
  typename std::enable_if_t< detail::is_shared_ptr< ValueType >::value,
    bool > = true,
  typename std::enable_if_t< std::is_base_of_v< kwiver::arrows::ceres::solver_options, typename ValueType::element_type >, bool > = true >
void
set_config_helper(
  vital::config_block_sptr config, const std::string& key,
  const ValueType& value,
  [[maybe_unused]] config_block_description_t const& description  =
  config_block_description_t() )
{
  if( value )
  {
    value->get_configuration( config );
  }
  // We only set a value to assign a description to the key.
  // The value will never be read from the config itself,
  // as this type has a custom specialized accessor
  // that returns a new instance each time.
  config->set_value< ValueType >( key, value, description );
}

/// A helper for retrieving a value from a config block. This specialization is
/// for keys that correspond to nested solver_options.
template < typename ValueType,
  typename std::enable_if_t< detail::is_shared_ptr< ValueType >::value,
    bool > = true,
  typename std::enable_if_t< std::is_base_of_v< kwiver::arrows::ceres::solver_options, typename ValueType::element_type >, bool > = true >
ValueType
get_config_helper(
  vital::config_block_sptr config,
  config_block_key_t const& key )
{
  ValueType solver_options = std::make_shared< kwiver::arrows::ceres::solver_options >();
  solver_options->set_configuration( config );
  return solver_options;
}

// ----------------------------------------------------------------------------
// specializations of set/get_config_helper for camera_options
// for base implementations see config_helpers.txx

/// A helper for populating \p key in \p config based on the configuration of
/// the camera_options given in \p value.
template < typename ValueType,
  typename std::enable_if_t< detail::is_shared_ptr< ValueType >::value,
    bool > = true,
  typename std::enable_if_t< std::is_base_of_v< kwiver::arrows::ceres::camera_options, typename ValueType::element_type >, bool > = true >
void
set_config_helper(
  vital::config_block_sptr config, const std::string& key,
  const ValueType& value,
  [[maybe_unused]] config_block_description_t const& description  =
  config_block_description_t() )
{
  if( value )
  {
    value->get_configuration( config );
  }
  // We only set a value to assign a description to the key.
  // The value will never be read from the config itself,
  // as this type has a custom specialized accessor
  // that returns a new instance each time.
  config->set_value< ValueType >( key, value, description );
}

/// A helper for retrieving a value from a config block. This specialization is
/// for keys that correspond to nested camera_options.
template < typename ValueType,
  typename std::enable_if_t< detail::is_shared_ptr< ValueType >::value,
    bool > = true,
  typename std::enable_if_t< std::is_base_of_v< kwiver::arrows::ceres::camera_options, typename ValueType::element_type >, bool > = true >
ValueType
get_config_helper(
  vital::config_block_sptr config,
  config_block_key_t const& key )
{
  ValueType camera_options = std::make_shared< kwiver::arrows::ceres::camera_options >();
  camera_options->set_configuration( config );

  return camera_options;
}

} // namespace kwiver::arrows::ceres

#endif
