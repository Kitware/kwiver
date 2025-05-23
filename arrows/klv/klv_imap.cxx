// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of KLV IMAP encoding / decoding logic.

#include <arrows/klv/klv_imap.h>

#include <arrows/klv/klv_util.h>
#include <arrows/klv/klv_read_write.txx>

#include <vital/logger/logger.h>

#include <iomanip>
#include <limits>
#include <tuple>

#include <cmath>
#include <cfloat>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
struct cmp_values
{
  klv_imap::kind_t kind;
  double value;
  uint64_t other_bits;
};

DEFINE_STRUCT_CMP(
  cmp_values,
  &cmp_values::kind,
  &cmp_values::value,
  &cmp_values::other_bits
)

// ----------------------------------------------------------------------------
// Helper struct.
struct _imap_terms
{
  double forward_scale;
  double backward_scale;
  double zero_offset;
};

// ----------------------------------------------------------------------------
// Calculates the derived terms needed for both IMAP reading and writing.
_imap_terms
_calculate_imap_terms(
  vital::interval< double > const& interval, size_t length )
{
  // ST1201, Section 8.1.2
  auto const float_exponent = std::ceil( std::log2( interval.span() ) );
  auto const int_exponent = 8.0 * length - 1.0;

  _imap_terms result = {};
  result.forward_scale = std::exp2( int_exponent - float_exponent );
  result.backward_scale = std::exp2( float_exponent - int_exponent );
  result.zero_offset = interval.contains( 0.0, false, false )
                       ? result.forward_scale * interval.lower() -
                       std::floor( result.forward_scale * interval.lower() )
                       : 0.0;
  return result;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
klv_imap
::klv_imap()
  : m_value{ 0 },
    m_other_bits{ 0 },
    m_kind{ KIND_NORMAL }
{}

// ----------------------------------------------------------------------------
klv_imap
::klv_imap( double value )
  : m_value{ value },
    m_other_bits{ 0 },
    m_kind{ KIND_NORMAL }
{
  if( std::isnan( value ) )
  {
    m_kind = KIND_NAN_QUIET;
  }
}

// ----------------------------------------------------------------------------
klv_imap
::klv_imap( double value, kind_t kind, uint64_t other_bits )
  : m_value{ value },
    m_other_bits{ other_bits },
    m_kind{ kind }
{
  // We need room for the five special header bits
  if( other_bits >= 1ull << 59 )
  {
    throw std::runtime_error( "IMAP other bits are too large" );
  }
}

// ----------------------------------------------------------------------------
klv_imap
klv_imap
::nan( bool is_signaling, bool sign, uint64_t nan_id )
{
  double value;
  if( is_signaling )
  {
    // No portable way to encode nan id
    value = std::numeric_limits< double >::signaling_NaN();
  }
  else
  {
    value = std::nan( std::to_string( nan_id ).c_str() );
  }

  if( sign )
  {
    value = -value;
  }

  auto const kind = is_signaling ? KIND_NAN_SIGNALING : KIND_NAN_QUIET;
  return klv_imap( value, kind, nan_id );
}

// ----------------------------------------------------------------------------
klv_imap
klv_imap
::below_minimum()
{
  return klv_imap(
    -std::numeric_limits< double >::infinity(), KIND_BELOW_MIN, 0 );
}

// ----------------------------------------------------------------------------
klv_imap
klv_imap
::above_maximum()
{
  return klv_imap(
    std::numeric_limits< double >::infinity(), KIND_ABOVE_MAX, 0 );
}

// ----------------------------------------------------------------------------
klv_imap
klv_imap
::user_defined( uint64_t payload )
{
  return klv_imap(
    std::numeric_limits< double >::quiet_NaN(), KIND_USER_DEFINED, payload );
}

// ----------------------------------------------------------------------------
klv_imap::kind_t
klv_imap
::kind() const
{
  return m_kind;
}

// ----------------------------------------------------------------------------
double
klv_imap
::as_double() const
{
  return m_value;
}

// ----------------------------------------------------------------------------
uint64_t
klv_imap
::other_bits() const
{
  return m_other_bits;
}

// ----------------------------------------------------------------------------
uint8_t
klv_imap
::other_bits_count() const
{
  return _int_bit_length( m_other_bits );
}

// ----------------------------------------------------------------------------
#define DEFINE_OPERATOR( OP )                                           \
bool                                                                    \
klv_imap                                                                \
::operator OP ( klv_imap const& other ) const                           \
{                                                                       \
  return cmp_values{ m_kind, m_value, m_other_bits } OP                 \
         cmp_values{ other.m_kind, other.m_value, other.m_other_bits }; \
}

DEFINE_OPERATOR( < )
DEFINE_OPERATOR( > )
DEFINE_OPERATOR( <= )
DEFINE_OPERATOR( >= )
DEFINE_OPERATOR( == )
DEFINE_OPERATOR( != )

#undef DEFINE_OPERATOR

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_imap const& value )
{
  switch( value.kind() )
  {
    case klv_imap::KIND_USER_DEFINED:
      os << "<user-defined(" << value.other_bits() << ")>";
      break;
    case klv_imap::KIND_NAN_QUIET:
      os << "<qnan(" << value.other_bits() << ")>";
      break;
    case klv_imap::KIND_NAN_SIGNALING:
      os << "<snan(" << value.other_bits() << ")>";
      break;
    case klv_imap::KIND_NORMAL:
      os << value.as_double();
      break;
    case klv_imap::KIND_BELOW_MIN:
      os << "<below-minimum>";
      break;
    case klv_imap::KIND_ABOVE_MAX:
      os << "<above-maximum>";
      break;
    default:
      os << "<invalid>";
  }
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_imap::kind_t const& value )
{
  static std::string strings[ klv_imap::KIND_ENUM_END + 1 ] = {
    "User Defined",
    "Quiet NaN",
    "Signaling NaN",
    "Below Minimum",
    "Normal",
    "Above Maximum",
    "Unknown IMAP Kind",
  };

  os << strings[ std::min( value, klv_imap::KIND_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
klv_imap_format
::klv_imap_format( vital::interval< double > const& interval,
                   klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }, m_interval{ interval }
{}

// ----------------------------------------------------------------------------
klv_lengthy< klv_imap >
klv_imap_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return { klv_read_imap( m_interval, data, length ), length };
}

// ----------------------------------------------------------------------------
void
klv_imap_format
::write_typed( klv_lengthy< klv_imap > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_imap( value.value, m_interval, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_imap_format
::length_of_typed( klv_lengthy< klv_imap > const& value ) const
{
  // Add 5 bits for the header, plus 7 so we round up when dividing
  size_t const min_length = ( value.value.other_bits_count() + 5u + 7u ) / 8u;

  auto const suggested_length =
    m_length_constraints.fixed_or( m_length_constraints.suggested() );

  return std::max( min_length, value.length ? value.length : suggested_length );
}

// ----------------------------------------------------------------------------
std::ostream&
klv_imap_format
::print_typed(
  std::ostream& os, klv_lengthy< klv_imap > const& value ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_length_constraints.fixed_or( value.length );
  auto const digits = length ? _bits_to_decimal_digits( length * 8 - 1 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_imap_format
::description_() const
{
  std::stringstream ss;
  ss << "Float (Encoding: IMAP) (Range: " << m_interval << ")";
  return ss.str();
}

// ----------------------------------------------------------------------------
vital::interval< double >
klv_imap_format
::interval() const
{
  return m_interval;
}

// ----------------------------------------------------------------------------
klv_imap
klv_read_imap(
  vital::interval< double > const& interval,
  klv_read_iter_t& data, size_t length )
{
  // Section 8.1.2
  _check_range_length( interval, length );

  auto const int_value = klv_read_int< uint64_t >( data, length );

  // Section 8.2.2
  // Left-shift required to shift a bit from the least significant place to the
  // most significant place
  auto const msb_shift = length * 8 - 1;

  // Most significant bit and any other bit set means this is a special value
  if( int_value & ( 1ull << msb_shift ) && int_value != ( 1ull << msb_shift ) )
  {
    auto other_bits_count = static_cast< uint8_t >( length * 8 - 5 );
    auto other_bits = int_value & ~( 0b0001'1111ull << other_bits_count );

    // Second - fifth most significant bits = special value identifiers
    auto const identifier = ( int_value >> other_bits_count ) & 0b1111;

    switch( identifier )
    {
      case 0b1001: // Positive infinity
        if( other_bits )
        {
          VITAL_THROW(
            kv::metadata_exception,
            "invalid +inf IMAP value: other bits not zero" );
        }
        return klv_imap{ std::numeric_limits< double >::infinity() };
      case 0b1101: // Negative infinity
        if( other_bits )
        {
          VITAL_THROW(
            kv::metadata_exception,
            "invalid -inf IMAP value: other bits not zero" );
        }
        return klv_imap{ -std::numeric_limits< double >::infinity() };
      case 0b1010: // Positive quiet NaN
        return klv_imap::nan( false, false, other_bits );
      case 0b1110: // Negative quiet NaN
        return klv_imap::nan( false, true, other_bits );
      case 0b1011: // Positive signaling NaN
        return klv_imap::nan( true, false, other_bits );
      case 0b1111: // Negative signaling NaN
        return klv_imap::nan( true, true, other_bits );
      case 0b1000: // User defined value
        return klv_imap::user_defined( other_bits );
      case 0b1100: // MISB special value
        {
          // Now the next three bits further determine which special value it is
          other_bits_count -= 3;
          other_bits &= ~( 0b0111ull << other_bits_count );

          auto const misb_special_bits =
            ( int_value >> other_bits_count ) & 0b0111;

          switch( misb_special_bits )
          {
            case 0b0000:
              if( other_bits )
              {
                VITAL_THROW(
                  kv::metadata_exception,
                  "invalid below_min IMAP value: other bits not zero" );
              }
              return klv_imap::below_minimum();
            case 0b0001:
              if( other_bits )
              {
                VITAL_THROW(
                  kv::metadata_exception,
                  "invalid above_max IMAP value: other bits not zero" );
              }
              return klv_imap::above_maximum();
            default:
              VITAL_THROW( kv::metadata_exception, "reserved IMAP value" );
          }
        }
      default:
        VITAL_THROW( kv::metadata_exception, "reserved IMAP value" );
    }
  }

  // Normal value
  auto const terms = _calculate_imap_terms( interval, length );
  auto value =
    terms.backward_scale *
    ( static_cast< double >( int_value ) - terms.zero_offset ) +
    interval.lower();

  // Return exactly zero if applicable, overriding rounding errors. IMAP
  // specification considers this important
  auto const precision = klv_imap_precision( interval, length );
  value = ( std::abs( value ) < precision / 2.0 ) ? 0.0 : value;

  if( !interval.contains( value, true, true ) )
  {
    VITAL_THROW( kv::metadata_type_overflow, "value outside IMAP bounds" );
  }

  return klv_imap{ value };
}

// ----------------------------------------------------------------------------
void
klv_write_imap( klv_imap value, vital::interval< double > const& interval,
                klv_write_iter_t& data, size_t length )
{
  // Section 8.1.2, 8.2.1
  _check_range_length( interval, length );

  if( length * 8 - 5 < value.other_bits_count() )
  {
    VITAL_THROW(
      kv::metadata_exception,
      "IMAP extra bits cannot fit in length given" );
  }

  uint64_t int_value = 0;
  size_t shift_amount = ( length - 1 ) * 8;
  switch( value.kind() )
  {
    case klv_imap::KIND_NORMAL:
    {
      // Check for infinities
      if( std::isinf( value.as_double() ) )
      {
        int_value = std::signbit( value.as_double() ) ? 0xE8ull : 0xC8ull;
        int_value <<= shift_amount;
        break;
      }

      // Check for below minimum
      if( value.as_double() < interval.lower() )
      {
        LOG_DEBUG(
          kv::get_logger( "klv" ),
          "Truncating IMAP"
          << "(" << interval.lower() << ", " << interval.upper() << ") "
          << "value " << value.as_double() << " to <below-minimum>" );

        // Same as KIND_BELOW_MIN
        int_value = 0xE0ull << shift_amount;
        break;
      }

      // Check for above maximum
      if( value.as_double() > interval.upper() )
      {
        LOG_DEBUG(
          kv::get_logger( "klv" ),
          "Truncating IMAP"
          << "(" << interval.lower() << ", " << interval.upper() << ") "
          << "value " << value.as_double() << " to <above-maximum>" );

        // Same as KIND_ABOVE_MAX
        int_value = 0xE1ull << shift_amount;
        break;
      }

      auto const terms = _calculate_imap_terms( interval, length );
      int_value =
        static_cast< uint64_t >(
          terms.forward_scale * ( value.as_double() - interval.lower() ) +
          terms.zero_offset );
      break;
    }
    case klv_imap::KIND_NAN_QUIET:
      int_value = std::signbit( value.as_double() ) ? 0xF0ull : 0xD0ull;
      int_value <<= shift_amount;
      int_value |= value.other_bits();
      break;
    case klv_imap::KIND_NAN_SIGNALING:
      int_value = std::signbit( value.as_double() ) ? 0xF8ull : 0xD8ull;
      int_value <<= shift_amount;
      int_value |= value.other_bits();
      break;
    case klv_imap::KIND_BELOW_MIN:
      int_value = 0xE0ull << shift_amount;
      break;
    case klv_imap::KIND_ABOVE_MAX:
      int_value = 0xE1ull << shift_amount;
      break;
    case klv_imap::KIND_USER_DEFINED:
      int_value = 0xC0ull << shift_amount;
      int_value |= value.other_bits();
      break;
    default:
      throw std::logic_error( "invalid IMAP kind" );
  }

  klv_write_int( int_value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_imap_length( vital::interval< double > const& interval, double precision )
{
  // ST1201, Section 8.1.1
  _check_range_precision( interval, precision );

  auto const length_bits = std::ceil( std::log2( interval.span() ) ) -
                           std::floor( std::log2( precision ) ) + 1.0;
  return static_cast< size_t >( std::ceil( length_bits / 8.0 ) );
}

// ----------------------------------------------------------------------------
double
klv_imap_precision( vital::interval< double > const& interval, size_t length )
{
  _check_range_length( interval, length );

  auto const length_bits = length * 8.0;
  return std::exp2( std::log2( interval.span() ) - length_bits + 1 );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
