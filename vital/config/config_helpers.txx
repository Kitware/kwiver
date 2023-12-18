// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef CONFIG_HELPERS_TXX
#define CONFIG_HELPERS_TXX

#include <memory>
#include <type_traits>

namespace kwiver::vital {

// A helper to detect when a type is a std::shared_ptr
namespace detail {

template < typename T >
struct is_shared_ptr : std::false_type {};

template < typename T >
struct is_shared_ptr< std::shared_ptr< T > >: std::true_type {};

} // namespace detail

// --------------------------------------------------------------------------------------------
// A helper for getting a value from a config block. This specialization is for
// all types but std::shared_ptr. The value is returned via \p value
template < typename ValueType,
           typename std::enable_if_t< !detail::is_shared_ptr< ValueType >::value, bool > = true >
void
set_config_helper( config_block_sptr config, const std::string& value_name,
                   const ValueType& value )
{
  config->set_value< ValueType >( value_name, value );
}

// --------------------------------------------------------------------------------------------
// A helper for getting a value to a config block. This specialization is for
// all types except std::shared_ptr
template < typename ValueType,
           typename std::enable_if_t< !detail::is_shared_ptr< ValueType >::value, bool > = true >
ValueType
get_config_helper( config_block_sptr config, config_block_key_t const& key )
{
  return config->get_value< ValueType >( key );
}

// --------------------------------------------------------------------------------------------
// A helper for getting a value from a config block. This specialization is for
// all types except std::shared_ptr with a default value
template < typename ValueType,
           typename std::enable_if_t< !detail::is_shared_ptr< ValueType >::value, bool > = true >
ValueType
get_config_helper( config_block_sptr config, config_block_key_t const& key,
                   ValueType const& default_value )
{
  return config->get_value< ValueType >( key, default_value );
}

} // namespace kwiver::vital

#endif
