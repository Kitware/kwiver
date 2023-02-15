// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of bytestream (de)compressor.

#ifndef KWIVER_ARROWS_ZLIB_BYTESTREAM_COMPRESSOR_H_
#define KWIVER_ARROWS_ZLIB_BYTESTREAM_COMPRESSOR_H_

#include <arrows/zlib/kwiver_algo_zlib_export.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <streambuf>
#include <vector>

#include <cstdint>
#include <cstdio>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Provides compression and decompression functionality.
class KWIVER_ALGO_ZLIB_EXPORT bytestream_compressor
{
public:
  // Operation to be performed on the input data.
  enum mode_t
  {
    MODE_COMPRESS,
    MODE_DECOMPRESS,
    MODE_ENUM_END,
  };

  /// Compression algorithm to use.
  enum compression_type_t
  {
    COMPRESSION_TYPE_DEFLATE,
    // COMPRESSION_TYPE_GZIP, // TODO
    COMPRESSION_TYPE_ENUM_END,
  };

  /// Nature of the uncompressed data, to help fine-tune the algorithm.
  enum data_type_t
  {
    DATA_TYPE_BINARY,
    DATA_TYPE_TEXT,
    DATA_TYPE_ENUM_END,
  };

  /// \throw std::runtime_error If given configuration is not available.
  bytestream_compressor(
    mode_t mode, compression_type_t compression_type, data_type_t data_type );

  bytestream_compressor( bytestream_compressor const& ) = delete;
  bytestream_compressor& operator=( bytestream_compressor const& ) = delete;

  bytestream_compressor( bytestream_compressor&& other );
  bytestream_compressor& operator=( bytestream_compressor&& other );

  ~bytestream_compressor();

  /// Give the data between \p begin and \p end to be (de)compressed.
  void write( void const* begin, void const* end );

  /// Give \p bytes to be (de)compressed.
  void write( std::vector< uint8_t > const& bytes );

  /// Write (de)compressed data to the buffer between \p begin and \p end.
  ///
  /// \note Some data may remain buffered internally. Use \c flush() to force
  //// all data to be readable.
  ///
  /// \return Pointer to byte one past the last one written.
  void* read( void* begin, void* end );

  /// Return all available (de)compressed data.
  ///
  /// \note Some data may remain buffered internally. Use \c flush() to force
  //// all data to be readable.
  std::vector< uint8_t > read();

  /// Return the number of currently available (de)compressed bytes.
  size_t readable_bytes() const;

  /// Finish (de)compression on remaining buffered data.
  ///
  /// After calling, all (de)compressed data will be available via \c read().
  ///
  /// \warning
  ///   Too-frequent use of this function may degrade quality of compression.
  void flush();

private:
  class impl;

  std::unique_ptr< impl > d;
};

// ----------------------------------------------------------------------------
/// Wrapper around another std::istream which (de)compresses the data as it
/// comes in.
template< class CharT, class Traits = std::char_traits< CharT > >
class basic_compress_istream
  : private std::basic_streambuf< CharT, Traits >,
    public std::basic_istream< CharT, Traits >
{
public:
  using istream_t = std::basic_istream< CharT, Traits >;
  using base_t = std::basic_streambuf< CharT, Traits >;

  basic_compress_istream(
    istream_t& source, bytestream_compressor& compressor )
    : std::basic_streambuf< CharT, Traits >(),
      std::basic_istream< CharT, Traits >(this),
      m_source( source ),
      m_compressor( compressor ),
      m_flushed( false )
  {}

protected:
  /// Called when the buffer of characters already decoded is empty, and the
  /// user is requesting more characters.
  int
  underflow() override
  {
    for( void* ptr = &*m_buffer.begin(); true; )
    {
      // If the compressor has more output, use that
      if( m_compressor.readable_bytes() )
      {
        ptr = m_compressor.read( ptr, &*m_buffer.end() );
        base_t::setg(
          &*m_buffer.begin(), &*m_buffer.begin(),
          static_cast< CharT* >( ptr ) );

        if( ptr == &*m_buffer.end() )
        {
          // The buffer is now full; return success.
          return Traits::to_int_type( m_buffer.front() );
        }
      }

      if( m_flushed )
      {
        // There's no more incoming data
        if( ptr == &*m_buffer.begin() )
        {
          // The buffer's empty; we're done here
          return Traits::eof();
        }
        else
        {
          // Return the final, partially-filled buffer
          return Traits::to_int_type( m_buffer.front() );
        }
      }

      // The compressor needs more input; pull it from the wrapped istream
      std::array< CharT, buffer_size > tmp_buffer;
      auto const count =
        m_source.read( &*tmp_buffer.begin(), tmp_buffer.size() ).gcount();
      if( count )
      {
        // We found more data to give to the compressor; do that
        m_compressor.write(
          &*tmp_buffer.begin(), &*tmp_buffer.begin() + count );
      }
      else
      {
        // The wrapped istream has no more data; tell the compressor to finish
        m_compressor.flush();
        m_flushed = true;
      }
    }
  }

private:
  static constexpr size_t buffer_size = 512;
  istream_t& m_source;
  bytestream_compressor& m_compressor;
  std::array< CharT, buffer_size > m_buffer;
  bool m_flushed;
};

using compress_istream = basic_compress_istream< char >;

// ----------------------------------------------------------------------------
/// Wrapper around another std::ostream which (de)compresses the data as it
/// goes out.
///
/// \warning
///   Input is not guaranteed to write to the wrapped stream immediately;
///   call \c flush() to guarantee this. Frequent use of \c flush()
///   ( or \c std::endl ) will degrade the quality of the compression.
template< class CharT, class Traits = std::char_traits< CharT > >
class basic_compress_ostream
  : private std::basic_streambuf< CharT, Traits >,
    public std::basic_ostream< CharT, Traits >
{
public:
  using ostream_t = std::basic_ostream< CharT, Traits >;
  using base_t = std::basic_streambuf< CharT, Traits >;

  basic_compress_ostream(
    ostream_t& destination, bytestream_compressor& compressor )
    : std::basic_streambuf< CharT, Traits >(),
      std::basic_ostream< CharT, Traits >(this),
      m_destination( destination ),
      m_compressor( compressor )
  {
    // Input goes in the buffer
    base_t::setp( &*m_buffer.begin(), &*m_buffer.end() );
  }

  ~basic_compress_ostream()
  {
    // Ensure all data is written out before deletion
    sync();
  }

protected:
  // Called when the input buffer is full, forcing writeout.
  int
  overflow( int ch )
  {
    // Give the buffer data to the compressor and clear it
    m_compressor.write( base_t::pbase(), base_t::pptr() );
    base_t::setp( &*m_buffer.begin(), &*m_buffer.end() );

    if( ch == Traits::eof() )
    {
      // Special eof character means we flush all data to wrapped ostream
      m_compressor.flush();
    }
    else
    {
      // Character passed in goes at the front of the newly-cleared buffer
      m_buffer.front() = ch;
      base_t::pbump( 1 );
    }

    // Write all data that the compressor gives us to wrapped ostream
    std::array< CharT, buffer_size > tmp_buffer;
    while( m_compressor.readable_bytes() )
    {
      auto const ptr =
        m_compressor.read( &*tmp_buffer.begin(), &*tmp_buffer.end() );

      m_destination.write(
        &*tmp_buffer.begin(),
        std::distance( &*tmp_buffer.begin(), static_cast< CharT* >( ptr ) ) );
    }

    return 0;
  }

  /// Called when \c flush() is called on this object. Forces all data to be
  /// written out regardless if the buffer is full.
  int
  sync() override
  {
    // Write all our data to the wrapped ostream
    if( overflow( Traits::eof() ) == Traits::eof() )
    {
      // An error occurred
      return -1;
    }

    // Force the wrapped ostream to write out all of its data
    m_destination.flush();

    return m_destination.bad() ? -1 : 0;
  }

private:
  static constexpr size_t buffer_size = 512;
  ostream_t& m_destination;
  bytestream_compressor& m_compressor;
  std::array< CharT, buffer_size > m_buffer;
};

using compress_ostream = basic_compress_ostream< char >;

} // namespace vital

} // namespace kwiver

#endif
