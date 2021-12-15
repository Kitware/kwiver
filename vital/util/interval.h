// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Definition of interval template class.

#ifndef KWIVER_VITAL_UTIL_INTERVAL_H_
#define KWIVER_VITAL_UTIL_INTERVAL_H_

#include <vital/util/numeric.h>
#include <vital/util/vital_util_export.h>

#include <algorithm>
#include <stdexcept>
#include <type_traits>

#include <cmath>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Represents a numerical interval with an upper and lower bound.
template < class T >
class interval
{
public:
  interval( T lower, T upper )
    : m_lower{ std::min( lower, upper ) },
      m_upper{ std::max( lower, upper ) }
  {
    assert_not_nan( lower );
    assert_not_nan( upper );
  }

  /// Return lower bound.
  T
  lower() const
  {
    return m_lower;
  }

  /// Return upper bound.
  T
  upper() const
  {
    return m_upper;
  }

  /// Ensure the lower bound is at least \p new_lower.
  ///
  /// \throws invalid_argument If \p new_lower is greater than the current
  /// upper bound.
  void
  truncate_lower( T new_lower )
  {
    assert_not_nan( new_lower );
    if( new_lower > m_upper )
    {
      throw std::invalid_argument(
              "interval.truncate_lower(): "
              "new_lower cannot be greater than current upper" );
    }
    m_lower = std::max( m_lower, new_lower );
  }

  /// Ensure the upper bound is at most \p new_upper.
  ///
  /// \throws invalid_argument If \p new_upper is less than the current lower
  /// bound.
  void
  truncate_upper( T new_upper )
  {
    assert_not_nan( new_upper );
    if( new_upper < m_lower )
    {
      throw std::invalid_argument(
              "interval.truncate_upper(): "
              "new_upper cannot be less than current lower" );
    }
    m_upper = std::min( m_upper, new_upper );
  }

  /// Ensure the interval contains \p value.
  void
  encompass( T value )
  {
    assert_not_nan( value );
    m_lower = std::min( m_lower, value );
    m_upper = std::max( m_upper, value );
  }

  /// Return \c true if \p value is within in this half-open interval.
  bool
  contains( T value ) const
  {
    return m_lower <= value && value < m_upper;
  }

  /// Return \c true if \p value is within in this interval.
  ///
  /// \param inclusive_lower Whether to return \c true if \p value is equal to
  /// the lower bound.
  /// \param inclusive_upper Whether to return \c true if \p value is equal to
  /// the upper bound.
  bool
  contains( T value, bool inclusive_lower, bool inclusive_upper ) const
  {
    return ( m_lower < value || ( inclusive_lower && m_lower == value ) ) &&
           ( value < m_upper || ( inclusive_upper && m_upper == value ) );
  }

private:
  static void
  assert_not_nan( T value )
  {
    if( kwiver::vital::isnan( value ) )
    {
      throw std::invalid_argument( "interval: cannot accept NaN value" );
    }
  }

  T m_lower;
  T m_upper;
};

// ----------------------------------------------------------------------------
template < class T >
bool
operator==( interval< T > const& lhs, interval< T > const& rhs )
{
  return lhs.lower() == rhs.lower() && lhs.upper() == rhs.upper();
}

// ----------------------------------------------------------------------------
template < class T >
bool
operator!=( interval< T > const& lhs, interval< T > const& rhs )
{
  return !( lhs == rhs );
}

} // namespace vital

} // namespace kwiver

#endif
