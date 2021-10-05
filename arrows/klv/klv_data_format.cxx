// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV data formats.

#include "klv_data_format.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_data_format
::klv_data_format( size_t fixed_length ) : m_fixed_length{ fixed_length }
{}

// ----------------------------------------------------------------------------
std::string
klv_data_format
::type_name() const
{
  return kwiver::vital::demangle( type().name() );
}

// ----------------------------------------------------------------------------
std::string
klv_data_format
::to_string( klv_value const& value ) const
{
  std::stringstream ss;
  print( ss, value );
  return ss.str();
}

// ----------------------------------------------------------------------------
std::string
klv_data_format
::length_description() const
{
  std::stringstream ss;
  if( m_fixed_length )
  {
    ss << "length " << m_fixed_length;
  }
  else
  {
    ss << "variable length";
  }
  return ss.str();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
