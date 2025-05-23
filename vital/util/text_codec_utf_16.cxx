// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of the UTF-16 text codec.

#include <vital/util/text_codec_utf_16.h>

#include <vital/util/text_codec_priv.h>

namespace kwiver {

namespace vital {

namespace {

constexpr char32_t first_non_bmp = 0x10000;
constexpr size_t surrogate_bits = 10;
constexpr char32_t first_surrogate_mask = 0b1111'1111'1100'0000'0000;
constexpr char32_t second_surrogate_mask = 0b0000'0000'0011'1111'1111;
constexpr char16_t surrogate_pattern_mask = 0b1111'1100'0000'0000;
constexpr char16_t surrogate_value_mask = 0b0000'0011'1111'1111;
constexpr char16_t first_surrogate_pattern = 0b1101'1000'0000'0000;
constexpr char16_t second_surrogate_pattern = 0b1101'1100'0000'0000;

// ----------------------------------------------------------------------------
std::pair< char16_t, char16_t >
split_words( char32_t c )
{
  if( c < first_non_bmp )
  {
    // Only one word required
    return { c, 0 };
  }

  // Two words required: subtract offset...
  c -= first_non_bmp;

  // ... and derive the two surrogate words
  return {
    first_surrogate_pattern |
    ( ( c & first_surrogate_mask ) >> surrogate_bits ),
    second_surrogate_pattern |
    ( c & second_surrogate_mask ) };
}

// ----------------------------------------------------------------------------
char32_t
combine_words( char16_t first, char16_t second )
{
  // Combine the two surrogate words, adding back offset
  return ( ( ( first & surrogate_value_mask ) << surrogate_bits ) |
           ( second & surrogate_value_mask ) ) + first_non_bmp;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char32_t const*, char* >
text_codec_utf_16
::encode(
  char32_t const* decoded_begin, char32_t const* decoded_end,
  char* encoded_begin, char* encoded_end ) const
{
  while( decoded_begin < decoded_end )
  {
    auto const c = *decoded_begin;

    ENCODE_CHECK_CODE_POINT( c );

    // Determine whether one or two words are required
    size_t const size = ( c < first_non_bmp ) ? 2 : 4;

    ENCODE_CHECK_WRITE_SPACE( size );

    // Derive the word(s)
    auto const words = split_words( c );

    // Write
    write_word( words.first, encoded_begin );
    if( words.second )
    {
      write_word( words.second, encoded_begin + 2 );
    }
    encoded_begin += size;

    // Proceed to the next character
    ++decoded_begin;
  }

  // Success
  return { DONE, decoded_begin, encoded_begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char const*, char32_t* >
text_codec_utf_16
::decode(
  char const* encoded_begin, char const* encoded_end,
  char32_t* decoded_begin, char32_t* decoded_end, bool has_true_end ) const
{
  while( encoded_begin < encoded_end )
  {
    // Assume one word to start
    size_t size = 2;

    DECODE_CHECK_READ_SPACE( size );

    // Read the first word
    char16_t first_word = read_word( encoded_begin );

    // Determine if we need to read a second word
    char16_t second_word;
    char32_t c;
    switch( first_word & surrogate_pattern_mask )
    {
      case first_surrogate_pattern: // Must read second word
        size = 4;
        DECODE_CHECK_READ_SPACE( size );

        // Read the second word
        second_word = read_word( encoded_begin + 2 );

        if( ( second_word & surrogate_pattern_mask ) !=
            second_surrogate_pattern )
        {
          // Invalid second word
          DECODE_HANDLE_ERROR;

          // Proceed to next word
          encoded_begin += 2;
          continue;
        }

        // Derive the code point from the two words
        c = combine_words( first_word, second_word );
        break;

      case second_surrogate_pattern: // First word is invalid
        DECODE_HANDLE_ERROR;

        // Proceed to next word
        encoded_begin += 2;
        continue;

      default: // First word is all we need
        c = first_word;
        break;
    }

    DECODE_WRITE( c, size );
  }

  return { DONE, encoded_begin, decoded_begin };
}

// ----------------------------------------------------------------------------
std::string
text_codec_utf_16_be
::name() const
{
  return "UTF-16BE";
}

// ----------------------------------------------------------------------------
std::string
text_codec_utf_16_le
::name() const
{
  return "UTF-16LE";
}

} // namespace vital

} // namespace kwiver
