// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of text transcoding capabilities.

#include <vital/util/text_codec_transcode.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
text_transcoder
::text_transcoder( text_codec const& src_codec, text_codec const& dst_codec )
  : m_src_codec{ &src_codec }, m_dst_codec{ &dst_codec },
    m_buffer_begin{ m_buffer }, m_buffer_end{ m_buffer }
{}

// ----------------------------------------------------------------------------
text_transcoder
::text_transcoder( text_transcoder const& other )
  : m_src_codec{ other.m_src_codec }, m_dst_codec{ other.m_dst_codec },
    m_buffer_begin{ m_buffer + ( other.m_buffer_begin - other.m_buffer ) },
    m_buffer_end{ m_buffer + ( other.m_buffer_end - other.m_buffer ) }
{
  std::copy( other.m_buffer_begin, other.m_buffer_end, m_buffer_begin );
}

// ----------------------------------------------------------------------------
text_transcoder
::text_transcoder( text_transcoder&& other )
  : m_src_codec{ other.m_src_codec }, m_dst_codec{ other.m_dst_codec },
    m_buffer_begin{ m_buffer + ( other.m_buffer_begin - other.m_buffer ) },
    m_buffer_end{ m_buffer + ( other.m_buffer_end - other.m_buffer ) }
{
  std::copy( other.m_buffer_begin, other.m_buffer_end, m_buffer_begin );
}

// ----------------------------------------------------------------------------
text_transcoder&
text_transcoder
::operator=( text_transcoder const& other )
{
  m_src_codec = other.m_src_codec;
  m_dst_codec = other.m_dst_codec;
  m_buffer_begin = m_buffer + ( other.m_buffer_begin - other.m_buffer );
  m_buffer_end = m_buffer + ( other.m_buffer_end - other.m_buffer );
  std::copy( other.m_buffer_begin, other.m_buffer_end, m_buffer_begin );
  return *this;
}

// ----------------------------------------------------------------------------
text_transcoder&
text_transcoder
::operator=( text_transcoder&& other )
{
  m_src_codec = other.m_src_codec;
  m_dst_codec = other.m_dst_codec;
  m_buffer_begin = m_buffer + ( other.m_buffer_begin - other.m_buffer );
  m_buffer_end = m_buffer + ( other.m_buffer_end - other.m_buffer );
  std::copy( other.m_buffer_begin, other.m_buffer_end, m_buffer_begin );
  return *this;
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, char const*, char* >
text_transcoder
::transcode(
  char const* src_begin, char const* src_end,
  char* dst_begin, char* dst_end, bool has_true_end )
{
  // Store decoded intermediate in BUFSIZ-sized chunks
  text_codec::result_code decode_code;
  text_codec::result_code encode_code;
  do
  {
    if( m_buffer_begin == m_buffer_end )
    {
      // Buffer is empty; decode more input
      m_buffer_begin = m_buffer;
      m_buffer_end = m_buffer + BUFSIZ;
      std::tie( decode_code, src_begin, m_buffer_end ) =
        m_src_codec->decode(
          src_begin, src_end, m_buffer_begin, m_buffer_end, has_true_end );
    }
    else
    {
      // Process remainder in buffer before decoding more input
      decode_code = text_codec::OUT_OF_SPACE;
    }

    // Encode data from the buffer to the output
    std::tie( encode_code,
              const_cast< char32_t const*& >( m_buffer_begin ),
              dst_begin ) =
      m_dst_codec->encode(
        m_buffer_begin, m_buffer_end, dst_begin, dst_end );
  } while( decode_code == text_codec::OUT_OF_SPACE &&
           encode_code == text_codec::DONE );

  text_codec::result_code code =
    ( decode_code == text_codec::ABORT ) ? text_codec::ABORT : encode_code;

  return { code, src_begin, dst_begin };
}

// ----------------------------------------------------------------------------
text_transcoder&
text_transcoder
::clear()
{
  m_buffer_begin = m_buffer;
  m_buffer_end = m_buffer;
  return *this;
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, std::string >
text_codec_transcode(
  text_codec const& src_codec, text_codec const& dst_codec,
  std::string const& s )
{
  // Build output in BUFSIZ-sized chunks
  char buffer[ BUFSIZ ];
  text_transcoder transcoder( src_codec, dst_codec );
  text_codec::result_code code;
  std::string result;
  char const* src_begin = &*s.begin();
  char const* src_end = &*s.end();
  do
  {
    char* dst_begin = buffer;
    char* dst_end = buffer + BUFSIZ;
    std::tie( code, src_begin, dst_end ) =
      transcoder.transcode( src_begin, src_end, dst_begin, dst_end, true );
    result.insert( result.end(), dst_begin, dst_end );
  } while( code == text_codec::OUT_OF_SPACE );

  return { code, result };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, size_t >
text_codec_transcoded_size(
  text_codec const& src_codec, text_codec const& dst_codec,
  char const* begin, char const* end, bool has_true_end )
{
  // Count output in BUFSIZ-sized chunks
  char buffer[ BUFSIZ ];
  text_transcoder transcoder( src_codec, dst_codec );
  text_codec::result_code code;
  size_t result = 0;
  do
  {
    char* dst_begin = buffer;
    char* dst_end = buffer + BUFSIZ;
    std::tie( code, begin, dst_end ) =
      transcoder.transcode( begin, end, dst_begin, dst_end, has_true_end );
    result += dst_end - dst_begin;
  } while( code == text_codec::OUT_OF_SPACE );

  return { code, result };
}

// ----------------------------------------------------------------------------
std::tuple< text_codec::result_code, size_t >
text_codec_transcoded_size(
  text_codec const& src_codec, text_codec const& dst_codec,
  std::string const& s )
{
  return text_codec_transcoded_size(
    src_codec, dst_codec, &*s.begin(), &*s.end(), true );
}

} // namespace vital

} // namespace kwiver
