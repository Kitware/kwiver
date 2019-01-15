/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#ifndef VITAL_RANGE_FILTER_H
#define VITAL_RANGE_FILTER_H

#include <vital/range/defs.h>

namespace kwiver {
namespace vital {
namespace range {

// ----------------------------------------------------------------------------
/// Filtering range adapter.
/**
 * This range adapter applies a filter to the elements of a range. When
 * iterating over the range, only elements which pass the filter (that is, the
 * filter functor returns \c true) will be seen.
 *
 * \par Example:
 * \code
 * namespace r = kwiver::vital::range;
 *
 * std::vector<int> values = { 1, 2, 3, 4, 5, 6, 7, 8 };
 * auto is_even = []( int x ){ return ( x % 2 ) == 0; };
 *
 * for ( auto x : values | r::filter( is_even ) )
 *   std::cout << x << std::endl;
 *
 * // Output:
 * //  2
 * //  4
 * //  6
 * //  8
 * \endcode
 */
template < typename Functor, typename Range >
class filter_view : public generic_view
{
protected:
  using range_iterator_t = typename range_ref< Range const >::iterator_t;

public:
  using value_t = typename range_ref< Range const >::value_t;
  using filter_function_t = Functor;

  filter_view( filter_view const& ) = default;

  class iterator
  {
  public:
    iterator() = default;
    iterator( iterator const& ) = default;
    iterator& operator=( iterator const& ) = default;

    bool operator!=( iterator const& other ) const
    { return m_iter != other.m_iter; }

    value_t operator*() const { return *m_iter; }

    iterator& operator++();

    operator bool() const { return m_iter != m_end; }

  protected:
    friend class filter_view;
    iterator( range_iterator_t const& iter,
              range_iterator_t const& end,
              filter_function_t const& func )
      : m_iter{ iter }, m_end{ end }, m_func( func ) {}

    range_iterator_t m_iter, m_end;
    filter_function_t m_func;
  };

  filter_view( Range const& range, filter_function_t func )
    : m_range{ range }, m_func{ func } {}

  iterator begin() const;

  iterator end() const
  { return { m_range.end(), m_range.end(), m_func }; }

protected:
  range_ref< Range const > m_range;
  filter_function_t m_func;
};

// ----------------------------------------------------------------------------
template < typename FilterFunction, typename Range >
typename filter_view< FilterFunction, Range >::iterator
filter_view< FilterFunction, Range >
::begin() const
{
  auto iter = iterator{ m_range.begin(), m_range.end(), m_func };
  return ( iter && m_func( *iter ) ? iter : ++iter );
}

// ----------------------------------------------------------------------------
template < typename FilterFunction, typename Range >
typename filter_view< FilterFunction, Range >::iterator&
filter_view< FilterFunction, Range >::iterator
::operator++()
{
  while ( m_iter != m_end )
  {
    ++m_iter;
    if ( m_iter != m_end && m_func( *m_iter ) ) break;
  }
  return *this;
}

///////////////////////////////////////////////////////////////////////////////

KWIVER_RANGE_ADAPTER_FUNCTION( filter )

} // namespace range
} // namespace vital
} // namespace kwiver

#endif
