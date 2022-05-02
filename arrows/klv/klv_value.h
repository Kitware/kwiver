// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the interface for the \c klv_value class.

#include <arrows/klv/klv_blob.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/any.h>

#include <sstream>

#ifndef KWIVER_ARROWS_KLV_KLV_VALUE_H_
# define KWIVER_ARROWS_KLV_KLV_VALUE_H_

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Exception indicating a \c klv_value container did not contain the requested
/// type.
class KWIVER_ALGO_KLV_EXPORT klv_bad_value_cast : public std::bad_cast
{
public:
  klv_bad_value_cast( std::type_info const& requested_type,
                      std::type_info const& actual_type );

  virtual ~klv_bad_value_cast() noexcept = default;

  char const* what() const noexcept override;

private:
  std::string m_message;
};

// ----------------------------------------------------------------------------
/// Type-erased container class for the values of KLV fields.
///
/// It is necessary for this class to exist separately from kwiver::vital::any
/// to enforce that all contained values support comparison and stringstream
/// operations. It also contains an optional embedded byte count - for some KLV
/// data formats, the length can vary to reflect the precision of the numerical
/// value. Knowing this precision may be desirable when performing
/// calculations or writing the value back to KLV.
class KWIVER_ALGO_KLV_EXPORT klv_value
{
public:
  klv_value();

  template < class T,
             typename = typename std::enable_if<
               !std::is_same< typename std::decay< T >::type,
                              klv_value >::value &&
               !std::is_same< typename std::decay< T >::type,
                              kwiver::vital::any >::value >::type >
  klv_value( T&& value );

  klv_value( klv_value const& other );

  klv_value( klv_value&& other );

  ~klv_value();

  klv_value&
  operator=( klv_value const& other );

  klv_value&
  operator=( klv_value&& other );

  template < class T >
  klv_value&
  operator=( T&& rhs );

  /// Swap the contents of this \c klv_value with another.
  klv_value&
  swap( klv_value& rhs ) noexcept;

  /// Create an \c any object with a copy of this value.
  kwiver::vital::any
  to_any() const;

  /// Check if the object contains no value.
  bool
  empty() const noexcept;

  /// Check if the object contains a value which is not of type \c klv_blob.
  bool
  valid() const noexcept;

  /// Remove any existing value.
  void
  clear() noexcept;

  /// Return type information for the contained value.
  std::type_info const& type() const noexcept;

  /// Return the demangled type name of the contained value.
  std::string type_name() const noexcept;

  /// Return a string representation of the contained value.
  std::string
  to_string() const;

  /// Return a reference to the contained value of type \c T.
  ///
  /// \throws klv_bad_value_cast If the object does not contain a value of type
  /// \c T.
  template < class T >
  T&
  get();

  /// Return a reference to the contained value of type \c T.
  ///
  /// \throws klv_bad_value_cast If the object does not contain a value of type
  /// \c T.
  template < class T >
  T const&
  get() const;

  /// Return a pointer to the contained value of type \c T, or \c nullptr on
  /// failure.
  template < class T >
  T*
  get_ptr() noexcept;

  /// Return a pointer to the contained value of type \c T, or \c nullptr on
  /// failure.
  template < class T >
  T const*
  get_ptr() const noexcept;

  friend KWIVER_ALGO_KLV_EXPORT bool
  operator<( klv_value const& lhs, klv_value const& rhs );

  friend KWIVER_ALGO_KLV_EXPORT bool
  operator==( klv_value const& lhs, klv_value const& rhs );

  friend KWIVER_ALGO_KLV_EXPORT std::ostream&
  operator<<( std::ostream& os, klv_value const& rhs );

private:
  // Abstract base class interfacing with the contained data type.
  class internal_base;

  // Type-specific implementation of the internal_base interface.
  template < class T > class internal_;

  std::unique_ptr< internal_base > m_item;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_value const& lhs, klv_value const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_value const& lhs, klv_value const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator!=( klv_value const& lhs, klv_value const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_value const& rhs );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
