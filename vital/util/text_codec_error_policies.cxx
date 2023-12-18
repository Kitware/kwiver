// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of standard text codec error policies.

#include <vital/util/text_codec_error_policies.h>

#include <vital/vital_config.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char* >
text_codec_encode_error_policy_skip
::handle(
  [[maybe_unused]] text_codec const& codec, [[maybe_unused]] char32_t c,
  char* begin, [[maybe_unused]] char* end ) const
{
  // Write nothing and continue
  return { result_code::DONE, begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char* >
text_codec_encode_error_policy_abort
::handle(
  [[maybe_unused]] text_codec const& codec, [[maybe_unused]] char32_t c,
  char* begin, [[maybe_unused]] char* end ) const
{
  // Write nothing and send abort signal
  return { result_code::ABORT, begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char* >
text_codec_encode_error_policy_substitute
::handle(
  text_codec const& codec, [[maybe_unused]] char32_t c,
  char* begin, char* end ) const
{
  // Check that substitute character is encodable to avoid recursive errors
  char32_t substitute = 0xFFFD; // Preferred Unicode version
  if( !codec.can_encode( substitute ) )
  {
    substitute = 0x1A; // Backup ASCII version
  }
  if( !codec.can_encode( substitute ) )
  {
    // Unlikely, but if neither substitute character is encodable, just skip
    return { result_code::DONE, begin };
  }

  // Write substitute character
  result_code code;
  char32_t* substitute_begin = &substitute;
  char32_t* substitute_end = substitute_begin + 1;
  std::tie( code, std::ignore, begin ) =
    codec.encode( substitute_begin, substitute_end, begin, end  );
  return { code, begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char* >
text_codec_encode_error_policy_unicode_escape
::handle( text_codec const& codec, char32_t c, char* begin, char* end ) const
{
  static auto const digits = U"0123456789ABCDEF";
  char32_t substitute16[ 6 ]; // \uXXXX
  char32_t substitute32[ 10 ]; // \UXXXXXXXX
  char32_t* substitute_begin;
  char32_t* substitute_end;
  if( c <= 0xFFFF )
  {
    // \uXXXX for small numbers
    substitute16[ 0 ] = U'\\';
    substitute16[ 1 ] = U'u';
    substitute_begin = substitute16;
    substitute_end = substitute16 + 6;
  }
  else
  {
    // \UXXXXXXXX for big numbers
    substitute32[ 0 ] = U'\\';
    substitute32[ 1 ] = U'U';
    substitute_begin = substitute32;
    substitute_end = substitute32 + 10;
  }

  // Write the hex code backwards
  for( auto ptr = substitute_end - 1; ptr > substitute_begin + 1; --ptr )
  {
    *ptr = digits[ c & 0xF ];
    c >>= 4;
  }

  if( !codec.can_encode( substitute_begin, substitute_end ) )
  {
    // Unlikely, but if the \[uU]XX... code is not encodable, just skip
    return { result_code::DONE, begin };
  }

  // Check encoded size first, so we either write the whole thing or none of it
  size_t size;
  std::tie( std::ignore, size ) =
    codec.encoded_size( substitute_begin, substitute_end );

  if( begin + size > end )
  {
    // \[uU]XX... code is too long for output buffer
    return { result_code::OUT_OF_SPACE, begin };
  }

  // Write \[uU]XX... code
  result_code code;
  std::tie( code, std::ignore, begin ) =
    codec.encode( substitute_begin, substitute_end, begin, end );

  return { code, begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char32_t* >
text_codec_decode_error_policy_skip
::handle(
  [[maybe_unused]] text_codec const& codec,
  char32_t* begin, [[maybe_unused]] char32_t* end ) const
{
  // Write nothing and continue
  return { result_code::DONE, begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char32_t* >
text_codec_decode_error_policy_abort
::handle(
  [[maybe_unused]] text_codec const& codec,
  char32_t* begin, [[maybe_unused]] char32_t* end ) const
{
  // Write nothing and send abort signal
  return { result_code::ABORT, begin };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char32_t* >
text_codec_decode_error_policy_substitute
::handle( text_codec const& codec, char32_t* begin, char32_t* end ) const
{
  // Determine which substitute character to write
  char32_t substitute = 0xFFFD; // Preferred Unicode version
  if( !codec.can_encode( substitute ) )
  {
    substitute = 0x1A; // Backup ASCII version
  }
  if( !codec.can_encode( substitute ) )
  {
    // Unlikely, but if neither substitute character is encodable, just skip
    return { result_code::DONE, begin };
  }

  // Check we have enough room to write it
  if( begin >= end )
  {
    return { result_code::OUT_OF_SPACE, begin };
  }

  // Write
  *begin = substitute;

  return { result_code::DONE, begin + 1 };
}

} // namespace vital

} // namespace kwiver
