// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Declaration of internal KLV utility functions.

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
operator<<( std::ostream& os, kwiver::vital::optional< T > const& value )
{
  if( value )
  {
    os << *value;
  }
  else
  {
    os << "(empty)";
  }
  return os;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
