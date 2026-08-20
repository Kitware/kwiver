// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of the ASCII text codec.

#include <vital/util/text_codec_ascii.h>

#include <vital/util/text_codec_priv.h>

#include <vital/vital_config.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
std::string
text_codec_ascii
::name() const
{
  return "ASCII";
}

// ----------------------------------------------------------------------------
bool
text_codec_ascii
::can_encode( char32_t c ) const
{
  return c <= 127;
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char32_t const*, char* >
text_codec_ascii
::encode(
  char32_t const* decoded_begin, char32_t const* decoded_end,
  char* encoded_begin, char* encoded_end ) const
{
  // Loop over input
  while( decoded_begin < decoded_end )
  {
    auto const c = *decoded_begin;

    ENCODE_CHECK_CODE_POINT( c );

    ENCODE_CHECK_WRITE_SPACE( 1 );

    // Write
    *encoded_begin = static_cast< char >( *decoded_begin );
    ++encoded_begin;

    // Proceed to next code point
    ++decoded_begin;
  }

  // Success
  return { DONE, decoded_begin, encoded_begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char const*, char32_t* >
text_codec_ascii
::decode(
  char const* encoded_begin, char const* encoded_end,
  char32_t* decoded_begin, char32_t* decoded_end,
  [[maybe_unused]] bool has_true_end ) const
{
  // Loop over input
  while( encoded_begin < encoded_end )
  {
    // Character is literal byte value
    char32_t const c = *encoded_begin;

    DECODE_WRITE( c, 1 );
  }

  // Success
  return { DONE, encoded_begin, decoded_begin };
}

} // namespace vital

} // namespace kwiver
