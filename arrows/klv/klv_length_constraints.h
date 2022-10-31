// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of KLV length constraints class.

#ifndef KWIVER_ARROWS_KLV_KLV_LENGTH_CONSTRAINTS_H_
#define KWIVER_ARROWS_KLV_KLV_LENGTH_CONSTRAINTS_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/optional.h>
#include <vital/util/interval.h>
#include <vital/util/variant/variant.hpp>

#include <set>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Description of how long a variable-length field is allowed to be.
class KWIVER_ALGO_KLV_EXPORT klv_length_constraints
{
public:
  /// Unconstrained length.
  klv_length_constraints();

  /// Length must be given value.
  klv_length_constraints( size_t fixed_length );

  /// Length must be between given values, inclusive on both ends.
  klv_length_constraints( size_t minimum, size_t maximum );
  klv_length_constraints( size_t minimum, size_t maximum, size_t suggested );

  /// Length must be one of supplied values.
  klv_length_constraints( std::set< size_t > const& allowed );
  klv_length_constraints( std::set< size_t > const& allowed,
                          size_t suggested );

  /// Returns whether \p length is an allowable length.
  bool
  do_allow( size_t length ) const;

  /// Returns \c true if the length is completely unconstrained.
  bool
  is_free() const;

  /// Returns the single value the length is fixed to, if it exists.
  vital::optional< size_t >
  fixed() const;

  /// Returns the fixed length, or \p backup if the length is not fixed.
  size_t
  fixed_or( size_t backup ) const;

  /// Returns the interval of allowed lengths, if it exists.
  vital::optional< vital::interval< size_t > >
  interval() const;

  /// Returns the set of allowed lengths, if it exists.
  vital::optional< std::set< size_t > >
  set() const;

  /// Return a suggested, valid length.
  size_t
  suggested() const;

  /// Set a custom suggestion.
  void
  set_suggested( size_t length );

  /// Textual description of the constraints.
  std::string
  description() const;

private:
  vital::variant<
    vital::monostate, size_t, vital::interval< size_t >, std::set< size_t >
    > m_impl;
  vital::optional< size_t > m_suggested;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
