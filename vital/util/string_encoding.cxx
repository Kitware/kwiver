// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of string encoding utilities.

#include <vital/util/string_encoding.h>

#include <stdexcept>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
size_t
utf8_code_point_count( char const* begin, char const* end )
{
  static std::runtime_error const invalid_error( "Invalid UTF-8" );

  size_t skip = 0;
  size_t count = 0;
  for( auto ptr = begin; ptr != end; ++ptr )
  {
    auto const c = *ptr;

    // Are we already in a multi-byte character?
    if( skip )
    {
      if( ( c & 0b1100'0000 ) != 0b1000'0000 )
      {
        // Continuing bytes should always start with 10xxxxxxx
        throw invalid_error;
      }

      // Don't count this byte as a new character
      --skip;
      continue;
    }

    // Must be the first byte of a new character
    // Iterate through the most significant five bits until we hit a zero
    for( int i = 7; i >= 3; --i )
    {
      if( c & ( 1 << i ) )
      {
        if( i == 3 )
        {
          // No byte should start with 11111xxx
          throw invalid_error;
        }
      }
      else if( i == 7 )
      {
        // Single-byte character (ASCII)
        break;
      }
      else if( i == 6 )
      {
        // 10xxxxxx should not start a character
        throw invalid_error;
      }
      else
      {
        // Start of a multi-byte character
        // The number of previous 1's determines the number of following bytes
        skip = 6 - i;
        break;
      }
    }

    ++count;
  }

  if( skip )
  {
    // String ends in the middle of a multi-byte character
    throw invalid_error;
  }

  return count;
}

// ----------------------------------------------------------------------------
size_t
utf8_code_point_count( std::string const& s )
{
  return utf8_code_point_count( &*s.cbegin(), &*s.cend() );
}

} // namespace vital

} // namespace kwiver
