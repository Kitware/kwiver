// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of the UTF-8 text codec.

#include <vital/util/text_codec_utf_8.h>

#include <vital/util/text_codec_priv.h>

#include <cstdint>

namespace kwiver {

namespace vital {

namespace {

// ----------------------------------------------------------------------------
constexpr size_t first_byte_options = 4;
constexpr uint8_t first_byte_masks[ first_byte_options ] =
{ 0b1000'0000, 0b1110'0000, 0b1111'0000, 0b1111'1000 };
constexpr uint8_t first_byte_patterns[ first_byte_options ] =
{ 0b0000'0000, 0b1100'0000, 0b1110'0000, 0b1111'0000 };
constexpr size_t first_byte_value_bits[ first_byte_options ] = { 7, 5, 4, 3 };
constexpr uint8_t continue_byte_mask = 0b1100'0000;
constexpr uint8_t continue_byte_pattern = 0b1000'0000;
constexpr size_t continue_byte_value_bits = 6;

// ----------------------------------------------------------------------------
size_t
continue_bytes_needed( char32_t c )
{
  return ( c >= U'\u0080' ) + ( c >= U'\u0800' ) + ( c >= U'\U00010000' );
}

// ----------------------------------------------------------------------------
size_t
continue_bytes_from_first_byte( char byte )
{
  size_t result = 0;
  while(
    result < first_byte_options &&
    ( static_cast< unsigned char >( byte ) & first_byte_masks[ result ] ) !=
    first_byte_patterns[ result ] )
  {
    ++result;
  }
  return result;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
std::string
text_codec_utf_8
::name() const
{
  return "UTF-8";
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char32_t const*, char* >
text_codec_utf_8
::encode(
  char32_t const* decoded_begin, char32_t const* decoded_end,
  char* encoded_begin, char* encoded_end ) const
{
  while( decoded_begin < decoded_end )
  {
    auto c = *decoded_begin;

    ENCODE_CHECK_CODE_POINT( c );

    // Determine number of continuation bytes
    auto const continue_bytes = continue_bytes_needed( c );

    ENCODE_CHECK_WRITE_SPACE( continue_bytes + 1 )

    // Write bytes in reverse
    for( size_t i = 0; i < continue_bytes; ++i )
    {
      encoded_begin[ continue_bytes - i ] =
        continue_byte_pattern | ( c & ~continue_byte_mask );
      c >>= continue_byte_value_bits;
    }

    // Write first byte last
    encoded_begin[ 0 ] =
      first_byte_patterns[ continue_bytes ] |
      ( c & ~first_byte_masks[ continue_bytes ] );
    encoded_begin += continue_bytes + 1;

    // Proceed to the next code point
    ++decoded_begin;
  }

  // Success
  return { DONE, decoded_begin, encoded_begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char const*, char32_t* >
text_codec_utf_8
::decode(
  char const* encoded_begin, char const* encoded_end,
  char32_t* decoded_begin, char32_t* decoded_end, bool has_true_end ) const
{
  while( encoded_begin < encoded_end )
  {
    // Determine number of continuation bytes
    auto const continue_bytes =
      continue_bytes_from_first_byte( encoded_begin[ 0 ] );

    if( continue_bytes == first_byte_options )
    {
      // Invalid starting byte
      DECODE_HANDLE_ERROR;

      // Proceed to next code point (byte)
      ++encoded_begin;
      continue;
    }

    // Read value bits from first byte
    char32_t c =
      static_cast< unsigned char >( encoded_begin[ 0 ] ) &
      ~first_byte_masks[ continue_bytes ];

    // Read continuation bytes
    for( size_t i = 1; i < continue_bytes + 1; ++i )
    {
      DECODE_CHECK_READ_SPACE( i );

      auto const byte = static_cast< unsigned char >( encoded_begin[ i ] );
      if( ( byte & continue_byte_mask ) != continue_byte_pattern ||
          byte == continue_byte_pattern )
      {
        // Invalid continuation byte
        DECODE_HANDLE_ERROR;

        // Proceed to the next byte
        encoded_begin += i + 1;
        continue;
      }

      // Success! Add these bits to the character
      c <<= continue_byte_value_bits;
      c |= byte & ~continue_byte_mask;
    }

    DECODE_WRITE( c, continue_bytes + 1 );
  }

  // Success
  return { DONE, encoded_begin, decoded_begin };
}

} // namespace vital

} // namespace kwiver
