// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of bytestream (de)compressor.

#include <arrows/zlib/bytestream_compressor.h>

#define ZLIB_CONST
#include <zlib.h>
#undef ZLIB_CONST

#include <array>
#include <deque>
#include <stdexcept>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
class bytestream_compressor::impl
{
public:
  impl( mode_t mode, compression_type_t compression_type,
        data_type_t data_type );

  ~impl();

  mode_t mode;
  compression_type_t compression_type;
  data_type_t data_type;
  z_stream_s stream;
  std::deque< uint8_t > buffer;
  bool flush;
};

// ----------------------------------------------------------------------------
bytestream_compressor::impl
::impl( mode_t mode, compression_type_t compression_type,
        data_type_t data_type )
  : mode{ mode }, compression_type{ compression_type }, data_type{ data_type },
    stream{}, flush{ false }
{
  if( mode >= MODE_ENUM_END ||
      compression_type >= COMPRESSION_TYPE_ENUM_END ||
      data_type >= DATA_TYPE_ENUM_END )
  {
    throw std::runtime_error( "Invalid arguments" );
  }

  switch( data_type )
  {
    case DATA_TYPE_TEXT:
      stream.data_type = Z_TEXT;
      break;
    case DATA_TYPE_BINARY:
      stream.data_type = Z_BINARY;
      break;
    default:
      throw std::logic_error( "Case not handled" );
      break;
  }

  switch( mode )
  {
    case MODE_COMPRESS:
      if( deflateInit( &stream, Z_BEST_COMPRESSION ) != Z_OK )
      {
        throw std::runtime_error{ "Initializing compression failed" };
      }
      break;
    case MODE_DECOMPRESS:
      if( inflateInit( &stream ) != Z_OK )
      {
        throw std::runtime_error{ "Initializing decompression failed" };
      }
      break;
    default:
      throw std::logic_error( "Case not handled" );
      break;
  }
}

// ----------------------------------------------------------------------------
bytestream_compressor::impl
::~impl()
{
  switch( mode )
  {
    case MODE_COMPRESS:
      deflateEnd( &stream );
      break;
    case MODE_DECOMPRESS:
      inflateEnd( &stream );
      break;
    default:
      break;
  }
}

// ----------------------------------------------------------------------------
bytestream_compressor
::bytestream_compressor(
  mode_t mode, compression_type_t compression_type, data_type_t data_type )
  : d{ new impl{ mode, compression_type, data_type } }
{}

// ----------------------------------------------------------------------------
bytestream_compressor
::bytestream_compressor( bytestream_compressor&& other )
  : d{}
{
  *this = std::move( other );
}

// ----------------------------------------------------------------------------
bytestream_compressor&
bytestream_compressor
::operator=( bytestream_compressor&& other )
{
  d = std::move( other.d );
  return *this;
}

// ----------------------------------------------------------------------------
bytestream_compressor
::~bytestream_compressor()
{}

// ----------------------------------------------------------------------------
void
bytestream_compressor
::write( void const* begin, void const* end )
{
  if( begin > end )
  {
    throw std::logic_error{ "Invalid range" };
  }

  int ( *fn )( z_streamp, int ) = nullptr;
  switch( d->mode )
  {
    case MODE_COMPRESS:
      fn = &deflate;
      break;
    case MODE_DECOMPRESS:
      fn = &inflate;
      break;
    default:
      throw std::logic_error( "Case not handled" );
      break;
  }

  auto const begin_byte = static_cast< Bytef const* >( begin );
  auto const end_byte = static_cast< Bytef const* >( end );
  d->stream.next_in = begin_byte;
  d->stream.avail_in =
    static_cast< uInt >( std::distance( begin_byte, end_byte ) );

  std::array< uint8_t, BUFSIZ > tmp_buffer;
  do
  {
    d->stream.next_out = &*tmp_buffer.begin();
    d->stream.avail_out = static_cast< uInt >( tmp_buffer.size() );

    auto const err = fn( &d->stream, d->flush );
    if( err != Z_OK && err != Z_STREAM_END && err != Z_BUF_ERROR )
    {
      throw std::runtime_error{ "(De)compression failed" };
    }
    d->buffer.insert(
        d->buffer.end(), &*tmp_buffer.begin(), d->stream.next_out );
  } while( d->stream.avail_out == 0 );

  d->flush = false;
}

// ----------------------------------------------------------------------------
void
bytestream_compressor
::write( std::vector< uint8_t > const& bytes )
{
  write( bytes.data(), bytes.data() + bytes.size() );
}

// ----------------------------------------------------------------------------
void*
bytestream_compressor
::read( void* begin, void* end )
{
  if( begin > end )
  {
    throw std::logic_error{ "Invalid range" };
  }

  auto const begin_byte = static_cast< Bytef* >( begin );
  auto const end_byte = static_cast< Bytef* >( end );

  // Copy data, careful not to overflow bounds
  auto const input_size = static_cast< size_t >( std::distance( begin_byte, end_byte ) );
  auto const copy_size = std::min( input_size, d->buffer.size() );
  auto const result = std::copy_n( d->buffer.begin(), copy_size, begin_byte );

  // Remove copied data
  d->buffer.erase( d->buffer.begin(), d->buffer.begin() + copy_size );

  return result;
}

// ----------------------------------------------------------------------------
std::vector< uint8_t >
bytestream_compressor
::read()
{
  std::vector< uint8_t > result{ d->buffer.begin(), d->buffer.end() };
  d->buffer.clear();
  return result;
}

// ----------------------------------------------------------------------------
size_t
bytestream_compressor
::readable_bytes() const
{
  return d->buffer.size();
}

// ----------------------------------------------------------------------------
void
bytestream_compressor
::flush()
{
  d->flush = true;
  write( {} );
}

} // namespace vital

} // namespace kwiver
