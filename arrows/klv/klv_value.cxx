// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the implementation of the \c klv_value class.

#include "klv_value.h"

#include "klv_blob.h"

#include <vital/util/demangle.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ---------------------------------------------------------------------------
klv_bad_value_cast
::klv_bad_value_cast( std::type_info const& requested_type,
                      std::type_info const& actual_type )
{
  std::stringstream ss;
  ss    << "klv_value: type "
        << kwiver::vital::demangle( requested_type.name() )
        << " was requested, but the object holds type "
        << kwiver::vital::demangle( actual_type.name() );
  m_message = ss.str();
}

// ---------------------------------------------------------------------------
char const*
klv_bad_value_cast
::what() const noexcept
{
  return m_message.c_str();
}

// ---------------------------------------------------------------------------
klv_value
::klv_value() : m_length_hint{ 0 } {}

// ---------------------------------------------------------------------------
klv_value
::klv_value( klv_value const& other ) : m_length_hint{ other.m_length_hint }
{
  m_item.reset( other.m_item ? other.m_item->clone() : nullptr );
}

// ---------------------------------------------------------------------------
klv_value
::klv_value( klv_value&& other )
{
  swap( other );
}

// ---------------------------------------------------------------------------
kwiver::vital::any
klv_value
::to_any() const
{
  return m_item ? m_item->to_any() : kwiver::vital::any{};
}

// ---------------------------------------------------------------------------
void
klv_value
::set_length_hint( size_t length_hint )
{
  m_length_hint = length_hint;
}

// ---------------------------------------------------------------------------
size_t
klv_value
::length_hint() const
{
  return m_length_hint;
}

// ---------------------------------------------------------------------------
klv_value&
klv_value
::swap( klv_value& rhs ) noexcept
{
  m_item.swap( rhs.m_item );
  std::swap( m_length_hint, rhs.m_length_hint );
  return *this;
}

// ---------------------------------------------------------------------------
bool
klv_value
::empty() const noexcept
{
  return !m_item;
}

// ---------------------------------------------------------------------------
bool
klv_value
::valid() const noexcept
{
  return m_item && type() != typeid( klv_blob );
}

// ---------------------------------------------------------------------------
void
klv_value
::clear() noexcept
{
  m_item.reset();
}

// ---------------------------------------------------------------------------
std::type_info const&
klv_value
::type() const noexcept
{
  return m_item ? m_item->type() : typeid( void );
}

// ---------------------------------------------------------------------------
std::string
klv_value
::type_name() const noexcept
{
  return kwiver::vital::demangle( type().name() );
}

// ---------------------------------------------------------------------------
std::string
klv_value
::to_string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

// ---------------------------------------------------------------------------
bool
operator<( klv_value const& lhs, klv_value const& rhs )
{
  if( rhs.empty() )
  {
    return false;
  }
  return lhs.empty() ? true : lhs.m_item->less_than( *rhs.m_item );
}

// ---------------------------------------------------------------------------
bool
operator==( klv_value const& lhs, klv_value const& rhs )
{
  if( lhs.empty() != rhs.empty() )
  {
    return false;
  }
  return ( lhs.empty() && rhs.empty() )
         ? true
         : lhs.m_item->equal_to( *rhs.m_item );
}

// ---------------------------------------------------------------------------
bool
operator!=( klv_value const& lhs, klv_value const& rhs )
{
  return !( lhs == rhs );
}

// ---------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_value const& rhs )
{
  return rhs.empty() ? os << "(empty)" : rhs.m_item->print( os );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
