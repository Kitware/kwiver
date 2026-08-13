// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// This file contains the interface for the \c klv_value class.

#ifndef KWIVER_ARROWS_KLV_KLV_VALUE_H
#define KWIVER_ARROWS_KLV_KLV_VALUE_H

#include <arrows/klv/klv_blob.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/any.h>

#include <memory>
#include <sstream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Exception indicating a \c klv_value container did not contain the requested
/// type.
class KWIVER_ALGO_KLV_EXPORT klv_bad_value_cast : public std::bad_cast
{
public:
  klv_bad_value_cast(
    std::type_info const& requested_type,
    std::type_info const& actual_type );

  virtual ~klv_bad_value_cast() noexcept= default;

  char const* what() const noexcept override;

private:
  std::string m_message;
};

// ----------------------------------------------------------------------------
/// Type-erased container class for the values of KLV fields.
///
/// This class can hold any KLV value, or be empty. Any type to be held in this
/// container must have comparison ( \c < ), equality ( \c == ), and
/// \c std::ostream ( \c << ) operators defined; these are passed on through
/// this container and allow basic generic operations such as sorting and
/// printing values to be performed without having to know the type of each
/// value at compile time.
///
/// Generally a KLV value is expected to be in one of three states: empty,
/// invalid, or valid. An empty value has no type and no data. An invalid value
/// has a type of \c klv_blob, and contains only raw bytes, usually the result
/// of failure to parse. A valid value has data of some other type which is
/// determined by the context in which the value exists.
///
/// \c klv_value is a relatively low-context data type in the KLV hierachy. The
/// type of a \c klv_value does not uniquely identify how that value is to be
/// interpreted or serialized; there are a handful of different ways of encoding
/// integers or floating-point numbers into KLV, for example. Classes derived
/// from \c klv_data_format deal with that next layer of specificity.
class KWIVER_ALGO_KLV_EXPORT klv_value
{
public:
  /// Construct an empty object.
  klv_value();

  /// Move some external type into a new object.
  template < class T,
    typename = std::enable_if_t<
      !std::is_same_v< std::decay_t< T >, klv_value > &&
      !std::is_same_v< std::decay_t< T >, vital::any > > >
  klv_value( T&& value );

  klv_value( klv_value const& other );
  klv_value( klv_value&& other );
  ~klv_value();

  klv_value&
  operator=( klv_value const& other );
  klv_value&
  operator=( klv_value&& other );

  /// Move some external type into this object.
  template < class T >
  klv_value&
  operator=( T&& rhs );

  /// Swap the contents of this object with \p rhs.
  klv_value&
  swap( klv_value& rhs ) noexcept;

  /// Create an \c any object with a copy of this value.
  vital::any
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
  /// \throws klv_bad_value_cast
  ///   If the object does not contain a value of type \c T.
  template < class T >
  T&
  get();

  /// \copydoc template < class T > T& get()
  template < class T >
  T const&
  get() const;

  /// Return a pointer to the contained value of type \c T, or \c nullptr on
  /// failure.
  template < class T >
  T*
  get_ptr() noexcept;

  /// \copydoc template < class T > T* get_ptr()
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
