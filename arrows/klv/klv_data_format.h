// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV data formats.

#ifndef KWIVER_ARROWS_KLV_KLV_DATA_FORMAT_H_
#define KWIVER_ARROWS_KLV_KLV_DATA_FORMAT_H_

#include "klv_blob.txx"
#include "klv_key.h"
#include "klv_lengthy.h"
#include "klv_read_write.txx"
#include "klv_uuid.hpp"
#include "klv_value.h"

#include <arrows/klv/kwiver_algo_klv_export.h>
#include <vital/exceptions/metadata.h>
#include <vital/logger/logger.h>

#include <memory>
#include <ostream>
#include <sstream>
#include <typeinfo>
#include <vector>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
using klv_read_iter_t = typename klv_bytes_t::const_iterator;
using klv_write_iter_t = typename klv_bytes_t::iterator;

// ----------------------------------------------------------------------------
class klv_checksum_packet_format;

// ----------------------------------------------------------------------------
/// Untyped base for KLV data formats.
///
/// This class provides an interface to the KLV data formats, providing read,
/// write, and printing capabilities.
class KWIVER_ALGO_KLV_EXPORT klv_data_format
{
public:
  /// \param fixed_length The exact length in bytes of this data format. If
  /// zero, the length is variable.
  explicit
  klv_data_format( size_t fixed_length );

  virtual
  ~klv_data_format() = default;

  /// Parse raw bytes into data type; return as \c klv_value.
  virtual klv_value
  read( klv_read_iter_t& data, size_t length ) const = 0;

  /// Write \c klv_value (holding proper type) to raw bytes.
  virtual void
  write( klv_value const& value, klv_write_iter_t& data,
         size_t length ) const = 0;

  /// Return number of bytes required to write \p value.
  ///
  /// \note The return value does not account for a checksum, if present.
  virtual size_t
  length_of( klv_value const& value ) const = 0;

  /// Return \c type_info for read / written type.
  virtual std::type_info const&
  type() const = 0;

  /// Return name of read / written type.
  std::string
  type_name() const;

  /// Print a string representation of \p value to \p os.
  virtual std::ostream&
  print( std::ostream& os, klv_value const& value ) const = 0;

  /// Return a string representation of \p value.
  std::string
  to_string( klv_value const& value ) const;

  /// Return a textual description of this data format.
  virtual std::string
  description() const = 0;

  /// Optionally the checksum format for this data format.
  virtual klv_checksum_packet_format const*
  checksum_format() const;

  /// Return the fixed length of this format, or 0 if length is variable.
  size_t
  fixed_length() const;

  /// Set the fixed length of this format.
  void
  set_fixed_length( size_t fixed_length );

protected:
  /// Describe the length of this data format.
  std::string
  length_description() const;

  size_t m_fixed_length;
};

using klv_data_format_sptr = std::shared_ptr< klv_data_format >;

// ----------------------------------------------------------------------------
/// Typed base for KLV data formats.
///
/// This class implements the functionality common to data formats of a
/// particular type. It takes care of checking for common edge cases like being
/// given empty data or invalid lengths, so specific derived data formats
/// don't need to duplicate that boilerplate in each class. Specific formats
/// only have to worry about overriding the \c *_typed() functions.
template < class T >
class KWIVER_ALGO_KLV_EXPORT klv_data_format_ : public klv_data_format
{
public:
  using data_type = T;

  explicit
  klv_data_format_( size_t fixed_length ) : klv_data_format{ fixed_length }
  {}

  virtual
  ~klv_data_format_() {}

  klv_value
  read( klv_read_iter_t& data, size_t length ) const override final
  {
    if( !length )
    {
      // Zero length: null / unknown value
      return klv_value{};
    }

    try
    {
      // Try to parse using this data format
      return read_( data, length );
    }
    catch ( std::exception const& e )
    {
      // Return blob if parsing failed
      LOG_ERROR( kwiver::vital::get_logger( "klv" ),
                "error occurred during parsing: " << e.what() );
      return klv_read_blob( data, length );
    }
  }

  T
  read_( klv_read_iter_t& data, size_t length ) const
  {
    if( !length )
    {
      VITAL_THROW( kwiver::vital::metadata_exception,
                  "zero length given to read_()" );
    }
    else if( m_fixed_length && length != m_fixed_length )
    {
      // Invalid length
      LOG_WARN( kwiver::vital::get_logger( "klv" ),
                "fixed-length format `" << description() <<
                "` received wrong number of bytes ( " << length << " )" );
    }

    return read_typed( data, length );
  }

  void
  write( klv_value const& value, klv_write_iter_t& data,
         size_t max_length ) const override final
  {
    if( value.empty() )
    {
      // Null / unknown value: write nothing
      return;
    }
    else if( !value.valid() )
    {
      // Unparsed value: write raw bytes
      klv_write_blob( value.get< klv_blob >(), data, max_length );
    }
    else
    {
      write_( value.get< T >(), data, max_length );
    }
  }

  void
  write_( T const& value, klv_write_iter_t& data, size_t max_length ) const
  {
    // Ensure we have enough bytes
    auto const value_length = length_of_( value );
    if( value_length > max_length )
    {
      VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                  "write will overflow buffer" );
    }

    // Write the value
    auto const begin = data;
    write_typed( value, data, value_length );

    // Ensure the number of bytes we wrote was how many planned to write
    auto const written_length =
      static_cast< size_t >( std::distance( begin, data ) );
    if( written_length != value_length )
    {
      std::stringstream ss;
      ss << "format `" << description() << "`: "
        << "written length (" << written_length << ") and "
        << "calculated length (" << value_length <<  ") not equal";
      throw std::logic_error( ss.str() );
    }
  }

  size_t
  length_of( klv_value const& value ) const override final
  {
    if( value.empty() )
    {
      return 0;
    }
    else if( !value.valid() )
    {
      return value.get< klv_blob >()->size();
    }
    else
    {
      return length_of_( value.get< T >() );
    }
  }

  size_t
  length_of_( T const& value ) const
  {
    return m_fixed_length ? m_fixed_length : length_of_typed( value );
  }

  std::type_info const&
  type() const override final
  {
    return typeid( T );
  }

  std::ostream&
  print( std::ostream& os, klv_value const& value ) const override final
  {
    return !value.valid()
          ? ( os << value )
          : print_( os, value.get< T >() );
  }

  std::ostream&
  print_( std::ostream& os, T const& value ) const
  {
    return print_typed( os, value );
  }

protected:
  // These functions are overridden by the specific data format classes.
  // length_of_typed() and print_typed() have default behavior provided to
  // make overriding optional.

  virtual T
  read_typed( klv_read_iter_t& data, size_t length ) const = 0;

  virtual void
  write_typed( T const& value, klv_write_iter_t& data,
               size_t length ) const = 0;

  virtual size_t
  length_of_typed( VITAL_UNUSED T const& value ) const
  {
    throw std::logic_error(
      std::string{} + "data format of type `" + type_name() +
      "` must either provide a fixed size or override length_of_typed()" );
  }

  virtual std::ostream&
  print_typed( std::ostream& os, T const& value ) const
  {
    if( std::is_same< T, std::string >::value )
    {
      return os << '"' << value << '"';
    }
    else
    {
      return os << value;
    }
  }
};

// ----------------------------------------------------------------------------
/// Treats data as a binary blob, or uninterpreted sequence of bytes.
class KWIVER_ALGO_KLV_EXPORT klv_blob_format
  : public klv_data_format_< klv_blob >
{
public:
  klv_blob_format( size_t fixed_length = 0 );

  virtual
  ~klv_blob_format() = default;

  std::string
  description() const override;

protected:
  klv_blob
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_blob const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_blob const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Treats data as a 16-byte UUID
class KWIVER_ALGO_KLV_EXPORT klv_uuid_format
  : public klv_data_format_< klv_uuid >
{
public:
  klv_uuid_format();

  std::string
  description() const override;

protected:
  klv_uuid
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_uuid const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_uuid const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a string.
class KWIVER_ALGO_KLV_EXPORT klv_string_format
  : public klv_data_format_< std::string >
{
public:
  klv_string_format( size_t fixed_length = 0 );

  std::string
  description() const override;

protected:
  std::string
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( std::string const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( std::string const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as an unsigned integer.
class KWIVER_ALGO_KLV_EXPORT klv_uint_format
  : public klv_data_format_< uint64_t >
{
public:
  klv_uint_format( size_t fixed_length = 0 );

  virtual
  ~klv_uint_format() = default;

  std::string
  description() const override;

protected:
  uint64_t
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( uint64_t const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( uint64_t const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a signed integer.
class KWIVER_ALGO_KLV_EXPORT klv_sint_format
  : public klv_data_format_< int64_t >
{
public:
  klv_sint_format( size_t fixed_length = 0 );

  virtual
  ~klv_sint_format() = default;

  std::string
  description() const override;

protected:
  int64_t
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( int64_t const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( int64_t const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as an enum type.
template < class T >
class KWIVER_ALGO_KLV_EXPORT klv_enum_format
  : public klv_data_format_< typename std::decay< T >::type >
{
public:
  using data_type = typename std::decay< T >::type;

  klv_enum_format( size_t fixed_length = 1 )
    : klv_data_format_< data_type >{ fixed_length }
  {}

  virtual
  ~klv_enum_format()
  {}

  std::string
  description() const override
  {
    std::stringstream ss;
    ss << this->type_name() << " enumeration of "
       << this->length_description();
    return ss.str();
  }

protected:
  data_type
  read_typed( klv_read_iter_t& data, size_t length ) const override
  {
    return static_cast< data_type >(
      klv_read_int< uint64_t >( data, length ) );
  }

  void
  write_typed( data_type const& value,
               klv_write_iter_t& data, size_t length ) const override
  {
    klv_write_int( static_cast< uint64_t >( value ), data, length );
  }

  size_t
  length_of_typed( data_type const& value ) const override
  {
    return klv_int_length( static_cast< uint64_t >( value ) );
  }

  size_t m_length;
};

// ----------------------------------------------------------------------------
/// Interprets data as an unsigned integer encoded in BER format.
class KWIVER_ALGO_KLV_EXPORT klv_ber_format
  : public klv_data_format_< uint64_t >
{
public:
  klv_ber_format();

  virtual
  ~klv_ber_format() = default;

  std::string
  description() const override;

protected:
  uint64_t
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( uint64_t const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( uint64_t const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as an unsigned integer encoded in BER-OID format.
class KWIVER_ALGO_KLV_EXPORT klv_ber_oid_format
  : public klv_data_format_< uint64_t >
{
public:
  klv_ber_oid_format();

  virtual
  ~klv_ber_oid_format() = default;

  std::string
  description() const override;

protected:
  uint64_t
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( uint64_t const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( uint64_t const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as IEEE-754 floating point value.
class KWIVER_ALGO_KLV_EXPORT klv_float_format
  : public klv_data_format_< klv_lengthy< double > >
{
public:
  klv_float_format( size_t fixed_length = 0 );

  virtual
  ~klv_float_format() = default;

  std::string
  description() const override;

protected:
  klv_lengthy< double >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_lengthy< double > const& value ) const override;

  std::ostream&
  print_typed( std::ostream& os,
               klv_lengthy< double > const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as an unsigned integer mapped to a known floating-point
/// range.
class KWIVER_ALGO_KLV_EXPORT klv_sflint_format
  : public klv_data_format_< klv_lengthy< double > >
{
public:
  klv_sflint_format( double minimum, double maximum, size_t fixed_length = 0 );

  virtual
  ~klv_sflint_format() = default;

  std::string
  description() const override;

  double
  minimum() const;

  double
  maximum() const;

protected:
  klv_lengthy< double >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_lengthy< double > const& value ) const override;

  std::ostream&
  print_typed( std::ostream& os,
               klv_lengthy< double > const& value ) const override;

  double m_minimum;
  double m_maximum;
};

// ----------------------------------------------------------------------------
/// Interprets data as an signed integer mapped to a known floating-point
/// range.
class KWIVER_ALGO_KLV_EXPORT klv_uflint_format
  : public klv_data_format_< klv_lengthy< double > >
{
public:
  klv_uflint_format( double minimum, double maximum, size_t fixed_length = 0 );

  virtual
  ~klv_uflint_format() = default;

  std::string
  description() const override;

  double
  minimum() const;

  double
  maximum() const;

protected:
  klv_lengthy< double >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_lengthy< double > const& value ) const override;

  std::ostream&
  print_typed( std::ostream& os,
               klv_lengthy< double > const& value ) const override;

  double m_minimum;
  double m_maximum;
};

// ----------------------------------------------------------------------------
/// Interprets data as a floating point value encoded in IMAP format.
class KWIVER_ALGO_KLV_EXPORT klv_imap_format
  : public klv_data_format_< klv_lengthy< double > >
{
public:
  klv_imap_format( double minimum, double maximum, size_t fixed_length = 0 );

  std::string
  description() const override;

  double
  minimum() const;

  double
  maximum() const;

protected:
  klv_lengthy< double >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_lengthy< double > const& value ) const override;

  std::ostream&
  print_typed( std::ostream& os,
               klv_lengthy< double > const& value ) const override;

  double m_minimum;
  double m_maximum;
};

// ----------------------------------------------------------------------------
template< class Format >
class KWIVER_ALGO_KLV_EXPORT klv_lengthless_format
  : public klv_data_format_< typename Format::data_type::value_type >
{
public:
  using data_type = typename Format::data_type::value_type;

  template< class... Args >
  klv_lengthless_format( Args&&... args )
    : klv_data_format_< data_type >{ 0 }, m_format{ args... } {
    if( !( this->m_fixed_length = m_format.fixed_length() ) )
    {
      throw std::logic_error( "klv_lengthless_format requires fixed length" );
    }
    m_format.set_fixed_length( 0 );
  }

  std::string
  description() const
  {
    return m_format.description();
  }

protected:
  data_type
  read_typed( klv_read_iter_t& data, size_t length ) const
  {
    return m_format.read_( data, length ).value;
  }

  void
  write_typed( data_type const& value,
               klv_write_iter_t& data, size_t length ) const
  {
    m_format.write_( { value, this->m_fixed_length }, data, length );
  }

  size_t
  length_of_typed( VITAL_UNUSED data_type const& value ) const
  {
    return this->m_fixed_length;
  }

  std::ostream&
  print_typed( std::ostream& os, data_type const& value ) const
  {
    return m_format.print_( os, { value, this->m_fixed_length } );
  }

  Format m_format;
};

// ----------------------------------------------------------------------------
template< class Enum, class Int >
std::set< Enum >
bitfield_to_enums( Int bitfield )
{
  static_assert( std::is_unsigned< Int >::value, "bitfield must be unsigned" );
  std::set< Enum > result;
  for( size_t i = 0; bitfield; ++i, bitfield >>= 1 )
  {
    if( bitfield & 1 )
    {
      result.emplace( static_cast< Enum >( i ) );
    }
  }
  return result;
}

// ----------------------------------------------------------------------------
template< class Enum, class Int = uint64_t >
Int
enums_to_bitfield( std::set< Enum > const& enums )
{
  static_assert( std::is_unsigned< Int >::value, "bitfield must be unsigned" );
  Int result = 0;
  for( auto const& element : enums )
  {
    result |= static_cast< Int >( 1 ) << static_cast< Int >( element );
  }
  return result;
}

// ----------------------------------------------------------------------------
template< class Enum, class Format = klv_uint_format >
class KWIVER_ALGO_KLV_EXPORT klv_enum_bitfield_format
  : public klv_data_format_< std::set< Enum > >
{
public:
  template< class... Args >
  klv_enum_bitfield_format( Args&&... args )
    : klv_data_format_< std::set< Enum > >{ 0 }, m_format{ args... } {
    this->m_fixed_length = m_format.fixed_length();
    m_format.set_fixed_length( 0 );
  }

  std::string
  description() const
  {
    return "bitfield of " + this->length_description();
  }

protected:
  std::set< Enum >
  read_typed( klv_read_iter_t& data, size_t length ) const
  {
    return bitfield_to_enums< Enum >( m_format.read_( data, length ) );
  }

  void
  write_typed( std::set< Enum > const& value,
               klv_write_iter_t& data, size_t length ) const
  {
    m_format.write_( enums_to_bitfield( value ), data, length );
  }

  size_t
  length_of_typed( std::set< Enum > const& value ) const
  {
    return m_format.length_of_( enums_to_bitfield( value ) );
  }

  Format m_format;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
