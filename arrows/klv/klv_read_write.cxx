// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the non-templated basic KLV read/write functions.

#include "klv_read_write.txx"

#include <stdexcept>

namespace kv = kwiver::vital;
#include <algorithm>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
uint64_t
_imap_infinity( bool sign_bit, size_t length )
{
  auto const identifier = sign_bit ? 0xE8ull : 0xC8ull;
  return length ? identifier << ( ( length - 1 ) * 8 ) : 0;
}

// ----------------------------------------------------------------------------
uint64_t
_imap_quiet_nan( bool sign_bit, size_t length )
{
  auto const identifier = sign_bit ? 0xF0ull : 0xD0ull;
  return length ? identifier << ( ( length - 1 ) * 8 ) : 0;
}

// ----------------------------------------------------------------------------
uint64_t
_imap_signal_nan( bool sign_bit, size_t length )
{
  auto const identifier = sign_bit ? 0xF8ull : 0xD8ull;
  return length ? identifier << ( ( length - 1 ) * 8 ) : 0;
}

// ----------------------------------------------------------------------------
_imap_terms
_calculate_imap_terms( double minimum, double maximum, size_t length )
{
  // ST1201, Section 8.1.2
  auto const float_exponent = std::ceil( std::log2( maximum - minimum ) );
  auto const int_exponent = 8.0 * length - 1.0;

  _imap_terms result = {};
  result.forward_scale = std::exp2( int_exponent - float_exponent );
  result.backward_scale = std::exp2( float_exponent - int_exponent );
  result.zero_offset = ( minimum < 0 && maximum > 0 )
                       ? result.forward_scale * minimum -
                       std::floor( result.forward_scale * minimum )
                       : 0.0;
  return result;
}

// ----------------------------------------------------------------------------
size_t
klv_imap_length( double minimum, double maximum, double precision )
{
  // ST1201, Section 8.1.1
  _check_range_precision( minimum, maximum, precision );

  auto const length_bits = std::ceil( std::log2( maximum - minimum ) ) -
                           std::floor( std::log2( precision ) ) + 1.0;
  return static_cast< size_t >( std::ceil( length_bits / 8.0 ) );
}

// ----------------------------------------------------------------------------
double
klv_imap_precision( double minimum, double maximum, size_t length )
{
  _check_range_length( minimum, maximum, length );

  auto const length_bits = length * 8.0;
  return std::exp2( std::log2( maximum - minimum ) - length_bits + 1 );
}

namespace {

// ----------------------------------------------------------------------------
void
_check_range( double minimum, double maximum )
{
  if( !std::isfinite( minimum ) )
  {
    throw std::logic_error( "minimum must be finite" );
  }

  if( !std::isfinite( maximum ) )
  {
    throw std::logic_error( "maximum must be finite" );
  }

  if( std::isinf( maximum - minimum ) )
  {
    VITAL_THROW( kv::metadata_type_overflow,
                 "span too large for double type" );
  }

  if( !( minimum < maximum ) )
  {
    throw std::logic_error( "minimum must be less than maximum" );
  }
}

} // namespace

// ----------------------------------------------------------------------------
void
_check_range_precision( double minimum, double maximum, double precision )
{
  _check_range( minimum, maximum );

  if( !std::isfinite( precision ) )
  {
    throw std::logic_error( "precision must be finite" );
  }

  if( !( precision < maximum - minimum ) )
  {
    throw std::logic_error( "precision must be less than min-max span" );
  }
}

// ----------------------------------------------------------------------------
void
_check_range_length( double minimum, double maximum, size_t length )
{
  _check_range( minimum, maximum );

  if( !length )
  {
    throw std::logic_error( "length must not be zero" );
  }

  if( length > sizeof( uint64_t ) )
  {
    VITAL_THROW( kv::metadata_type_overflow,
                 "value too large for native type" );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_flint_length( double minimum, double maximum, double precision )
{
  _check_range_precision( minimum, maximum, precision );

  // Based on IMAP, but without the one extra bit IMAP uses for NaN, Inf
  auto const length_bits = std::ceil( std::log2( maximum - minimum ) ) -
                           std::floor( std::log2( precision ) );
  return static_cast< size_t >( std::ceil( length_bits / 8.0 ) );
}

// ----------------------------------------------------------------------------
double
klv_flint_precision( double minimum, double maximum, size_t length )
{
  _check_range_length( minimum, maximum, length );

  // Based on IMAP, but without the one extra bit IMAP uses for NaN, Inf
  auto const length_bits = length * 8.0;
  return std::exp2( std::log2( maximum - minimum ) - length_bits );
}

// ----------------------------------------------------------------------------
size_t
klv_string_length( std::string const& value )
{
  // "\0" is reserved for empty string
  // We avoid constructing a temp string object to compare against
  if( value.size() == 1 && value[ 0 ] == '\0' )
  {
    VITAL_THROW( kwiver::vital::metadata_type_overflow,
                 "the string \"\\0\" cannot be written to KLV stream" );
  }

  return std::max< size_t >( value.size(), 1 );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
