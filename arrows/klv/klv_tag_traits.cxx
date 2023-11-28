// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV tag traits interface.

#include "klv_tag_traits.h"

#include <algorithm>
#include <limits>
#include <sstream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_tag_count_range
::klv_tag_count_range( size_t exact ) : m_lower{ exact }, m_upper{ exact } {}

// ----------------------------------------------------------------------------
klv_tag_count_range
::klv_tag_count_range( size_t lower, size_t upper )
  : m_lower{ std::min( lower, upper ) }, m_upper{ std::max( lower, upper ) }
{}

// ----------------------------------------------------------------------------
size_t
klv_tag_count_range
::lower() const { return m_lower; }

// ----------------------------------------------------------------------------
size_t
klv_tag_count_range
::upper() const { return m_upper; }

// ----------------------------------------------------------------------------
bool
klv_tag_count_range
::is_count_allowed( size_t count ) const
{
  return m_lower <= count && count <= m_upper;
}

// ----------------------------------------------------------------------------
std::string
klv_tag_count_range
::description() const
{
  std::stringstream ss;
  if( m_lower == m_upper )
  {
    ss << "exactly " << m_lower;
  }
  else if( m_lower == 0 && m_upper == SIZE_MAX )
  {
    ss << "any number";
  }
  else if( m_lower == 0 )
  {
    ss << "at most " << m_upper;
  }
  else if( m_upper == SIZE_MAX )
  {
    ss << "at least " << m_lower;
  }
  else
  {
    ss << "between " << m_lower << " and " << m_upper;
  }
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_tag_traits
::klv_tag_traits( klv_uds_key uds_key,
                  klv_lds_key tag,
                  std::string const& enum_name,
                  klv_data_format_sptr format,
                  std::string const& name,
                  std::string const& description,
                  klv_tag_count_range const& tag_count_range,
                  klv_tag_traits_lookup const* subtag_lookup )
  : m_name{ name }, m_enum_name{ enum_name }, m_description{ description },
    m_lds_key{ tag }, m_uds_key{ uds_key }, m_format{ format },
    m_tag_count_range{ tag_count_range }, m_subtag_lookup{ subtag_lookup }
{}

// ----------------------------------------------------------------------------
klv_lds_key
klv_tag_traits
::tag() const { return m_lds_key; }

// ----------------------------------------------------------------------------
klv_uds_key
klv_tag_traits
::uds_key() const { return m_uds_key; }

// ----------------------------------------------------------------------------
std::string
klv_tag_traits
::name() const { return m_name; }

// ----------------------------------------------------------------------------
std::string
klv_tag_traits
::enum_name() const { return m_enum_name; }

// ----------------------------------------------------------------------------
std::type_info const&
klv_tag_traits
::type() const { return m_format->type(); }

// ----------------------------------------------------------------------------
std::string
klv_tag_traits
::type_name() const { return m_format->type_name(); }

// ----------------------------------------------------------------------------
std::string
klv_tag_traits
::description() const { return m_description; }

// ----------------------------------------------------------------------------
klv_data_format&
klv_tag_traits
::format() const { return *m_format; }

// ----------------------------------------------------------------------------
klv_tag_count_range
klv_tag_traits
::tag_count_range() const { return m_tag_count_range; }

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const*
klv_tag_traits
::subtag_lookup() const { return m_subtag_lookup; }

// ----------------------------------------------------------------------------
klv_tag_traits_lookup
::klv_tag_traits_lookup()
  : m_traits{}
{}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup
::klv_tag_traits_lookup( std::initializer_list< klv_tag_traits > const& traits )
  : m_traits{ traits.begin(), traits.end() }
{
  initialize();
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup
::klv_tag_traits_lookup( std::vector< klv_tag_traits > const& traits )
  : m_traits{ traits.begin(), traits.end() }
{
  initialize();
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup
::klv_tag_traits_lookup( klv_tag_traits_lookup const& other )
  : m_traits( other.m_traits )
{
  initialize();
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup&
klv_tag_traits_lookup
::operator=( klv_tag_traits_lookup const& other )
{
  m_tag_to_traits.clear();
  m_uds_key_to_traits.clear();
  m_name_to_traits.clear();
  m_enum_name_to_traits.clear();
  m_traits = other.m_traits;
  initialize();
  return *this;
}

// ----------------------------------------------------------------------------
typename klv_tag_traits_lookup::iterator
klv_tag_traits_lookup
::begin() const
{
  return m_traits.begin();
}

// ----------------------------------------------------------------------------
typename klv_tag_traits_lookup::iterator
klv_tag_traits_lookup
::end() const
{
  return m_traits.end();
}

// ----------------------------------------------------------------------------
klv_tag_traits const&
klv_tag_traits_lookup
::by_tag( klv_lds_key tag ) const
{
  auto const result = m_tag_to_traits.find( tag );
  return ( result == m_tag_to_traits.end() )
         ? m_traits.at( 0 )
         : *result->second;
}

// ----------------------------------------------------------------------------
klv_tag_traits const&
klv_tag_traits_lookup
::by_uds_key( klv_uds_key const& key ) const
{
  auto const result = m_uds_key_to_traits.find( key );
  return ( result == m_uds_key_to_traits.end() )
         ? m_traits.at( 0 )
         : *result->second;
}

// ----------------------------------------------------------------------------
klv_tag_traits const&
klv_tag_traits_lookup
::by_name( std::string const& name ) const
{
  auto const result = m_name_to_traits.find( name );
  return ( result == m_name_to_traits.end() )
         ? m_traits.at( 0 )
         : *result->second;
}

// ----------------------------------------------------------------------------
klv_tag_traits const&
klv_tag_traits_lookup
::by_enum_name( std::string const& enum_name ) const
{
  auto const result = m_enum_name_to_traits.find( enum_name );
  return ( result == m_enum_name_to_traits.end() )
         ? m_traits.at( 0 )
         : *result->second;
}

// ----------------------------------------------------------------------------
void
klv_tag_traits_lookup
::initialize()
{
  for( auto const& trait : m_traits )
  {
    if( trait.tag() && !m_tag_to_traits.emplace( trait.tag(), &trait ).second )
    {
      std::stringstream ss;
      ss << "duplicate tag in traits: " << trait.tag();
      throw std::logic_error( ss.str() );
    }
    if( trait.uds_key().is_valid() &&
        !m_uds_key_to_traits.emplace( trait.uds_key(), &trait ).second )
    {
      std::stringstream ss;
      ss << "duplicate UDS key in traits: " << trait.uds_key();
      throw std::logic_error( ss.str() );
    }
    if( !trait.name().empty() &&
        !m_name_to_traits.emplace( trait.name(), &trait ).second )
    {
      std::stringstream ss;
      ss << "duplicate name in traits: '" << trait.name() << "'";
      throw std::logic_error( ss.str() );
    }
    if( !trait.enum_name().empty() &&
        !m_enum_name_to_traits.emplace( trait.enum_name(), &trait ).second )
    {
      std::stringstream ss;
      ss << "duplicate enum name in traits: '" << trait.enum_name() << "'";
      throw std::logic_error( ss.str() );
    }
  }
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
