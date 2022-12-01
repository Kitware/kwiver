// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test bytestream_compressor class.

#include <gtest/gtest.h>

#include <arrows/zlib/bytestream_compressor.h>

#include <array>
#include <random>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class test_bytestream_compressor : public ::testing::Test
{
private:

  void
  SetUp() override
  {
    text_data.resize( data_size );
    binary_data.resize( data_size );

    // Create some repeating, compressable data
    for( size_t i = 0; i < data_size; ++i )
    {
      text_data[ i ] = 'A' + ( i % 16 ) + i / ( 1 << 14 );
      binary_data[ i ] = i % 64 + i / ( 1 << 12 );
    }
  }

public:
  constexpr static size_t data_size = 1 << 16; // 64 KB
  std::vector< uint8_t > text_data;
  std::vector< uint8_t > binary_data;
};

// ----------------------------------------------------------------------------
TEST_F ( test_bytestream_compressor, round_trip_deflate_text )
{
  bytestream_compressor compressor(
    bytestream_compressor::MODE_COMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_TEXT );

  compressor.write( text_data );
  compressor.flush();
  auto const compressed_data = compressor.read();

  EXPECT_EQ( 175, compressed_data.size() );

  bytestream_compressor decompressor(
    bytestream_compressor::MODE_DECOMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_TEXT );

  decompressor.write( compressed_data );
  decompressor.flush();
  auto const decompressed_data = decompressor.read();

  EXPECT_EQ( text_data, decompressed_data );
}

// ----------------------------------------------------------------------------
TEST_F ( test_bytestream_compressor, round_trip_deflate_binary )
{
  bytestream_compressor compressor(
    bytestream_compressor::MODE_COMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_BINARY );

  compressor.write( binary_data );
  compressor.flush();
  auto const compressed_data = compressor.read();

  EXPECT_EQ( 343, compressed_data.size() );

  bytestream_compressor decompressor(
    bytestream_compressor::MODE_DECOMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_BINARY );

  decompressor.write( compressed_data );
  decompressor.flush();
  auto const decompressed_data = decompressor.read();

  EXPECT_EQ( binary_data, decompressed_data );
}

// ----------------------------------------------------------------------------
TEST_F ( test_bytestream_compressor, round_trip_iostream_wrapper )
{
  bytestream_compressor compressor(
    bytestream_compressor::MODE_COMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_TEXT );

  bytestream_compressor decompressor(
    bytestream_compressor::MODE_DECOMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_TEXT );

  std::stringstream ss;
  compress_ostream compress_os( ss, compressor );
  compress_istream compress_is( ss, decompressor );

  std::string text( text_data.begin(), text_data.end() );
  compress_os << text << std::flush;

  EXPECT_EQ( 175, ss.str().size() );

  std::string out_text;
  compress_is >> out_text;
  std::vector< uint8_t > decompressed_data( out_text.begin(), out_text.end() );

  EXPECT_EQ( text_data, decompressed_data );
}

// ----------------------------------------------------------------------------
TEST_F ( test_bytestream_compressor, round_trip_deflate_piecemeal )
{
  // Feed the data to the (de)compressor in these small, prime-number-sized
  // chunks, instead of all at once
  constexpr size_t step = 37;

  bytestream_compressor compressor(
    bytestream_compressor::MODE_COMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_TEXT );

  for( size_t i = 0; i < text_data.size(); i += step )
  {
    compressor.write(
        text_data.data() + i,
        text_data.data() + std::min( i + step, text_data.size() ) );
  }
  compressor.flush();
  auto const compressed_data = compressor.read();

  EXPECT_EQ( 175, compressed_data.size() );

  bytestream_compressor decompressor(
    bytestream_compressor::MODE_DECOMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_TEXT );

  for( size_t i = 0; i < compressed_data.size(); i += step )
  {
    compressor.write(
        compressed_data.data() + i,
        compressed_data.data() +
        std::min( i + step, compressed_data.size() ) );
  }
  decompressor.write( compressed_data );
  decompressor.flush();
  auto const decompressed_data = decompressor.read();

  EXPECT_EQ( text_data, decompressed_data );
}

// ----------------------------------------------------------------------------
TEST_F ( test_bytestream_compressor, round_trip_iostream_wrapper_piecemeal )
{
  // Feed the data to the (de)compressor in these small, prime-number-sized
  // chunks, instead of all at once
  constexpr size_t step = 37;

  bytestream_compressor compressor(
    bytestream_compressor::MODE_COMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_TEXT );

  bytestream_compressor decompressor(
    bytestream_compressor::MODE_DECOMPRESS,
    bytestream_compressor::COMPRESSION_TYPE_DEFLATE,
    bytestream_compressor::DATA_TYPE_TEXT );

  std::stringstream ss;
  compress_ostream compress_os( ss, compressor );
  compress_istream compress_is( ss, decompressor );

  std::string text( text_data.begin(), text_data.end() );
  for( size_t i = 0; i < text.size(); i += step )
  {
    compress_os << text.substr( i, step );
  }
  compress_os << std::flush;

  EXPECT_EQ( 175, ss.str().size() );

  std::string out_text;
  for( size_t i = 0; i < text.size(); i += step )
  {
    std::string tmp( std::min( step, text.size() - i ), '\0' );
    compress_is.read( &tmp[ 0 ], tmp.size() );
    out_text += tmp;
  }
  std::vector< uint8_t > decompressed_data( out_text.begin(), out_text.end() );

  EXPECT_EQ( text_data, decompressed_data );
}
