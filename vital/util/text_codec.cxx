// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of general text codec functions.

#include <vital/util/text_codec.h>

#include <vital/util/text_codec_error_policies.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
text_codec
::text_codec()
  : m_encode_error_policy{ &default_encode_error_policy() },
    m_decode_error_policy{ &default_decode_error_policy() }
{}

// ----------------------------------------------------------------------------
text_codec
::~text_codec()
{}

// ----------------------------------------------------------------------------
bool
text_codec
::can_encode( char32_t const* begin, char32_t const* end ) const
{
  for( auto ptr = begin; ptr != end; ++ptr )
  {
    if( !can_encode( *ptr ) )
    {
      return false;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
bool
text_codec
::can_encode( std::u32string const& s ) const
{
  return can_encode( &*s.begin(), &*s.end() );
}

// ----------------------------------------------------------------------------
bool
text_codec
::can_encode( char32_t c ) const
{
  // Exclude code points above 0x10FFFF and surrogate code points.
  return c < 0x110000 && ( c < 0xD800 || c > 0xDFFF );
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, std::string >
text_codec
::encode( std::u32string const& s ) const
{
  // Build output in BUFSIZ-sized chunks
  char buffer[ BUFSIZ ];
  result_code code;
  std::string result;
  char32_t const* decoded_begin = &*s.begin();
  char32_t const* decoded_end = &*s.end();
  do
  {
    char* encoded_begin = buffer;
    char* encoded_end = buffer + BUFSIZ;
    std::tie( code, decoded_begin, encoded_end ) =
      encode( decoded_begin, decoded_end, encoded_begin, encoded_end );
    result.insert( result.end(), encoded_begin, encoded_end );
  } while( code == OUT_OF_SPACE );

  return { code, result };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, std::u32string >
text_codec
::decode( std::string const& s ) const
{
  // Build output in BUFSIZ-sized chunks
  char32_t buffer[ BUFSIZ ];
  result_code code;
  std::u32string result;
  char const* encoded_begin = &*s.begin();
  char const* encoded_end = &*s.end();
  do
  {
    char32_t* decoded_begin = buffer;
    char32_t* decoded_end = buffer + BUFSIZ;
    std::tie( code, encoded_begin, decoded_end ) =
      decode( encoded_begin, encoded_end,
              decoded_begin, decoded_end, true );
    result.insert( result.end(), decoded_begin, decoded_end );
  } while( code == OUT_OF_SPACE );

  return { code, result };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, size_t >
text_codec
::encoded_size( char32_t c ) const
{
  return encoded_size( &c, &c + 1 );
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, size_t >
text_codec
::encoded_size( char32_t const* begin, char32_t const* end ) const
{
  // Encode the sequence, then count and discard the output
  // Particular codecs may override this function for better performance
  char buffer[ BUFSIZ ];
  result_code code;
  size_t result = 0;
  do
  {
    char* buffer_begin = buffer;
    char* buffer_end = buffer + BUFSIZ;
    std::tie( code, begin, buffer_end ) =
      encode( begin, end, buffer_begin, buffer_end );
    result += buffer_end - buffer_begin;
  } while( code == OUT_OF_SPACE );

  return { code, result };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, size_t >
text_codec
::encoded_size( std::u32string const& s ) const
{
  return encoded_size( &*s.begin(), &*s.end() );
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, size_t >
text_codec
::decoded_size( char const* begin, char const* end, bool has_true_end ) const
{
  // Decode the sequence, then count and discard the output
  // Particular codecs may override this function for better performance
  char32_t buffer[ BUFSIZ ];
  result_code code;
  size_t result = 0;
  do
  {
    char32_t* buffer_begin = buffer;
    char32_t* buffer_end = buffer + BUFSIZ;
    std::tie( code, begin, buffer_end ) =
      decode( begin, end, buffer_begin, buffer_end, has_true_end );
    result += buffer_end - buffer_begin;
  } while( code == OUT_OF_SPACE );

  return { code, result };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, size_t >
text_codec
::decoded_size( std::string const& s ) const
{
  return decoded_size( &*s.begin(), &*s.end(), true );
}

// ----------------------------------------------------------------------------
void
text_codec
::set_encode_error_policy( encode_error_policy const& policy )
{
  m_encode_error_policy = &policy;
}

// ----------------------------------------------------------------------------
void
text_codec
::set_decode_error_policy( decode_error_policy const& policy )
{
  m_decode_error_policy = &policy;
}

// ----------------------------------------------------------------------------
text_codec::encode_error_policy const&
text_codec
::default_encode_error_policy()
{
  return text_codec_encode_error_policy_substitute::instance();
}

// ----------------------------------------------------------------------------
text_codec::decode_error_policy const&
text_codec
::default_decode_error_policy()
{
  return text_codec_decode_error_policy_substitute::instance();
}

} // namespace vital

} // namespace kwiver
