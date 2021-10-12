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
#define KWIVER_ARROWS_KLV_KLV_VALUE_H_

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

  template < class T, typename std::enable_if<
               !std::is_same< typename std::decay< T >::type,
                              kwiver::vital::any >::value &&
               !std::is_same< typename std::decay< T >::type,
                              klv_value >::value, bool >::type = true >
  klv_value( T&& value ) : m_length_hint{ 0 }
  {
    m_item.reset( new internal_< typename std::decay< T >::type >{ value } );
  }

  template < class T > explicit
  klv_value( T&& value, size_t length_hint )
  {
    klv_value{ value }.swap( *this );
    m_length_hint = length_hint;
  }

  klv_value( klv_value const& other );

  klv_value( klv_value&& other );

  ~klv_value() = default;

  klv_value&
  operator=( klv_value const& ) = default;

  template < class T >
  klv_value&
  operator=( T&& rhs )
  {
    klv_value{ rhs }.swap( *this );
    return *this;
  }

  /// Swap the contents of this \c klv_value with another.
  klv_value&
  swap( klv_value& rhs ) noexcept;

  /// Create an \c any object with a copy of this value.
  kwiver::vital::any
  to_any() const;

  /// Set the number of bytes this value should be written with.
  void
  set_length_hint( size_t length_hint );

  /// Get the number of bytes this value should be written with.
  size_t
  length_hint() const;

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
  get()
  {
    auto const ptr = get_ptr< T >();
    if( !ptr )
    {
      throw klv_bad_value_cast( typeid( T ), type() );
    }
    return *ptr;
  }

  /// Return a reference to the contained value of type \c T.
  ///
  /// \throws klv_bad_value_cast If the object does not contain a value of type
  /// \c T.
  template < class T >
  T const&
  get() const
  {
    auto const ptr = get_ptr< T >();
    if( !ptr )
    {
      throw klv_bad_value_cast( typeid( T ), type() );
    }
    return *ptr;
  }

  /// Return a pointer to the contained value of type \c T, or \c nullptr on
  /// failure.
  template < class T >
  T*
  get_ptr() noexcept
  {
    if( !m_item )
    {
      return nullptr;
    }

    auto const ptr = dynamic_cast< internal_< T >* >( m_item.get() );
    return ptr ? &ptr->m_item : nullptr;
  }

  /// Return a pointer to the contained value of type \c T, or \c nullptr on
  /// failure.
  template < class T >
  T const*
  get_ptr() const noexcept
  {
    if( !m_item )
    {
      return nullptr;
    }

    auto const ptr = dynamic_cast< internal_< T > const* >( m_item.get() );
    return ptr ? &ptr->m_item : nullptr;
  }

  friend KWIVER_ALGO_KLV_EXPORT bool
  operator<( klv_value const& lhs, klv_value const& rhs );

  friend KWIVER_ALGO_KLV_EXPORT bool
  operator==( klv_value const& lhs, klv_value const& rhs );

  friend KWIVER_ALGO_KLV_EXPORT std::ostream&
  operator<<( std::ostream& os, klv_value const& rhs );

private:

  // Abstract base class interfacing with the contained data type.
  class KWIVER_ALGO_KLV_EXPORT internal_base
  {
  public:
    virtual ~internal_base() = default;

    virtual std::type_info const& type() const noexcept = 0;
    virtual bool less_than( internal_base const& rhs ) const = 0;
    virtual bool equal_to( internal_base const& rhs ) const = 0;
    virtual std::ostream& print( std::ostream& os ) const = 0;
    virtual internal_base* clone() const = 0;
    virtual kwiver::vital::any to_any() const = 0;
  };

  // Type-specific implementation of the internal_base interface.
  template < class T >
  class KWIVER_ALGO_KLV_EXPORT internal_ : public internal_base
  {
  public:
    explicit
    internal_( T const& value ) : m_item( value ) {}

    explicit
    internal_( T&& value ) : m_item( value ) {}

    std::type_info const&
    type() const noexcept override final
    {
      return typeid( T );
    }

    bool
    less_than( internal_base const& rhs ) const override final
    {
      auto const& lhs = *this;
      // First, compare types
      if( lhs.type().before( rhs.type() ) )
      {
        return true;
      }
      else if( lhs.type() == rhs.type() )
      {
        auto const& rhs_item =
          dynamic_cast< internal_< T > const& >( rhs ).m_item;
        // Second, compare values
        return lhs.m_item < rhs_item;
      }
      return false;
    }

    bool
    equal_to( internal_base const& rhs ) const override final
    {
      auto const& lhs = *this;

      // First, compare types
      if( lhs.type() != rhs.type() )
      {
        return false;
      }

      auto const& rhs_item =
        dynamic_cast< internal_< T > const& >( rhs ).m_item;
      // Second, compare values
      return lhs.m_item == rhs_item;
    }

    std::ostream&
    print( std::ostream& os ) const override final
    {
      return os << m_item;
    }

    internal_base*
    clone() const override final
    {
      return new internal_< T >{ m_item };
    }

    kwiver::vital::any
    to_any() const override final
    {
      return m_item;
    }

    T m_item;
  };

  std::unique_ptr< internal_base > m_item;
  size_t m_length_hint;
};

// ---------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_value const& lhs, klv_value const& rhs );

// ---------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_value const& lhs, klv_value const& rhs );

// ---------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator!=( klv_value const& lhs, klv_value const& rhs );

// ---------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_value const& rhs );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
