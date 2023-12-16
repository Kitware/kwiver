// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of utilities to map 16- and 32-bit text encodings to bytes.

#ifndef KWIVER_VITAL_UTIL_TEXT_CODEC_ENDIAN_H_
#define KWIVER_VITAL_UTIL_TEXT_CODEC_ENDIAN_H_

#include <vital/util/vital_util_export.h>

#include <type_traits>

#include <cstdint>

namespace kwiver {

namespace vital {

static_assert( sizeof( char16_t ) == 2 );
static_assert( sizeof( char32_t ) == 4 );

// ----------------------------------------------------------------------------
template < class Word >
class VITAL_UTIL_EXPORT text_codec_endianness
{
public:
  /// Interpret \c word_size \c char, starting at \p begin, as a \c Word.
  ///
  /// \warning No range checking is performed.
  virtual Word read_word( char const* begin ) const = 0;

  /// Write \p word as \c word_size \c char, starting at \p begin.
  ///
  /// \warning No range checking is performed.
  virtual void write_word( Word word, char* begin ) const = 0;
};

using text_codec_16 = text_codec_endianness< char16_t >;
using text_codec_32 = text_codec_endianness< char32_t >;

// ----------------------------------------------------------------------------
/// Translates between \c Word and \c char in a big-endian fashion.
template < class Word >
class VITAL_UTIL_EXPORT text_codec_big_endian
  : virtual public text_codec_endianness< Word >
{
public:
  Word
  read_word( char const* begin ) const override final
  {
    static_assert( std::is_unsigned_v< Word > );

    Word result = 0;
    for( size_t i = 0; i < sizeof( Word ); ++i )
    {
      result <<= 8u;
      result |= static_cast< unsigned char >( begin[ i ] );
    }
    return result;
  }

  void
  write_word( Word word, char* begin ) const override final
  {
    static_assert( std::is_unsigned_v< Word > );
    for( size_t i = 0; i < sizeof( Word ); ++i )
    {
      begin[ sizeof( Word ) - i - 1 ] = static_cast< char >( word & 0xFFu );
      word >>= 8u;
    }
  }
};
using text_codec_16_be = text_codec_big_endian< char16_t >;
using text_codec_32_be = text_codec_big_endian< char32_t >;

// ----------------------------------------------------------------------------
/// Translates between \c Word and \c char in a little-endian fashion.
template < class Word >
class VITAL_UTIL_EXPORT text_codec_little_endian
  : virtual public text_codec_endianness< Word >
{
public:
  Word
  read_word( char const* begin ) const override final
  {
    static_assert( std::is_unsigned_v< Word > );

    Word result = 0;
    for( size_t i = 0; i < sizeof( Word ); ++i )
    {
      result <<= 8u;
      result |=
        static_cast< unsigned char >( begin[ sizeof( Word ) - i - 1 ] );
    }
    return result;
  }

  void
  write_word( Word word, char* begin ) const override final
  {
    static_assert( std::is_unsigned_v< Word > );
    for( size_t i = 0; i < sizeof( Word ); ++i )
    {
      begin[ i ] = static_cast< char >( word & 0xFFu );
      word >>= 8u;
    }
  }
};
using text_codec_16_le = text_codec_little_endian< char16_t >;
using text_codec_32_le = text_codec_little_endian< char32_t >;

} // namespace vital

} // namespace kwiver

#endif
