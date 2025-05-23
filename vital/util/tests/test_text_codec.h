// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utilities for testing text codecs.

#ifndef KWIVER_VITAL_UTIL_TESTS_TEST_TEXT_CODEC_H_
#define KWIVER_VITAL_UTIL_TESTS_TEST_TEXT_CODEC_H_

#include <vital/util/text_codec.h>

#include <tests/test_gtest.h>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
void
test_codec_encode(
  text_codec const& codec, std::u32string const& s32, std::string const& s )
{
  auto const encoded_size = codec.encoded_size( s32 );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, s.size() ), encoded_size );

  auto const encoded = codec.encode( s32 );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, s ), encoded );

  // Test on fixed-size buffer
  std::string buffer( s.size(), '\0' );
  auto const buffer_encoded =
    codec.encode(
        &*s32.begin(), &*s32.end(), &*buffer.begin(), &*buffer.end() );
  EXPECT_EQ(
    std::make_tuple( text_codec::DONE, &*s32.end(), &*buffer.end() ),
    buffer_encoded );
}

// ----------------------------------------------------------------------------
void
test_codec_decode(
  text_codec const& codec, std::string const& s, std::u32string const& s32,
  bool has_true_end = true )
{
  auto const decoded_size = codec.decoded_size( s );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, s32.size() ), decoded_size );

  auto const decoded = codec.decode( s );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, s32 ), decoded );

  // Test on fixed-size buffer
  std::u32string buffer( s32.size(), U'\0' );
  auto const buffer_decoded =
    codec.decode(
        &*s.begin(), &*s.end(), &*buffer.begin(), &*buffer.end(),
        has_true_end );
  EXPECT_EQ(
    std::make_tuple( text_codec::DONE, &*s.end(), &*buffer.end() ),
    buffer_decoded );
}

// ----------------------------------------------------------------------------
void
test_codec_round_trip(
  text_codec const& codec, std::string const& s, std::u32string const& s32 )
{
  auto const decoded_size = codec.decoded_size( s );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, s32.size() ), decoded_size );

  auto const decoded = codec.decode( s );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, s32 ), decoded );

  // Test on fixed-size buffer
  std::u32string buffer32( s32.size(), U'\0' );
  auto const buffer32_decoded =
    codec.decode(
        &*s.begin(), &*s.end(), &*buffer32.begin(), &*buffer32.end(), true );
  EXPECT_EQ(
    std::make_tuple( text_codec::DONE, &*s.end(), &*buffer32.end() ),
    buffer32_decoded );

  auto const encoded_size = codec.encoded_size( std::get< 1 >( decoded ) );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, s.size() ), encoded_size );

  auto const encoded = codec.encode( std::get< 1 >( decoded ) );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, s ), encoded );

  // Test on fixed-size buffer
  std::string buffer( s.size(), '\0' );
  auto const buffer_encoded =
    codec.encode(
        &*s32.begin(), &*s32.end(), &*buffer.begin(), &*buffer.end() );
  EXPECT_EQ(
    std::make_tuple( text_codec::DONE, &*s32.end(), &*buffer.end() ),
    buffer_encoded );
}

// ----------------------------------------------------------------------------
void
test_codec_encode_abort(
  text_codec const& codec, std::u32string const& s32, std::string const& s )
{
  auto const encoded_size = codec.encoded_size( s32 );
  EXPECT_EQ( std::make_tuple( text_codec::ABORT, s.size() ), encoded_size );

  auto const encoded = codec.encode( s32 );
  EXPECT_EQ( std::make_tuple( text_codec::ABORT, s ), encoded );
}

// ----------------------------------------------------------------------------
void
test_codec_decode_abort(
  text_codec const& codec, std::string const& s, std::u32string const& s32 )
{
  auto const decoded_size = codec.decoded_size( s );
  EXPECT_EQ( std::make_tuple( text_codec::ABORT, s32.size() ), decoded_size );

  auto const decoded = codec.decode( s );
  EXPECT_EQ( std::make_tuple( text_codec::ABORT, s32 ), decoded );
}

// ----------------------------------------------------------------------------
void
test_codec_encode_out_of_space(
  text_codec const& codec, std::u32string const& s32, std::string const& s,
  size_t output_limit, size_t input_distance )
{
  std::string output( output_limit, '\0' );
  auto const encoded =
    codec.encode(
        &*s32.begin(), &*s32.end(), &*output.begin(), &*output.end() );
  EXPECT_EQ(
    std::make_tuple(
      text_codec::OUT_OF_SPACE,
      &*s32.begin() + input_distance,
      &*output.begin() + s.size() ),
    encoded );
  EXPECT_EQ( s, output.substr( 0, s.size() ) );
  EXPECT_EQ(
    std::string( output_limit - s.size(), '\0' ), output.substr( s.size() ) );
}

// ----------------------------------------------------------------------------
void
test_codec_decode_out_of_space(
  text_codec const& codec, std::string const& s, std::u32string const& s32,
  size_t output_limit, size_t input_distance, bool has_true_end = true )
{
  std::u32string output( output_limit, U'\0' );
  auto const decoded =
    codec.decode(
        &*s.begin(), &*s.end(), &*output.begin(), &*output.end(),
        has_true_end );
  EXPECT_EQ(
    std::make_tuple(
      text_codec::OUT_OF_SPACE,
      &*s.begin() + input_distance, &*output.begin() + s32.size() ), decoded );
  EXPECT_EQ( s32, output.substr( 0, s32.size() ) );
  EXPECT_EQ(
    std::u32string( output_limit - s32.size(), U'\0' ),
    output.substr( s32.size() ) );
}

// ----------------------------------------------------------------------------
void
test_codec_invalid_ranges( text_codec const& codec )
{
  std::string s( "\0A", 2 );
  std::u32string s32( U"A" );

  EXPECT_EQ(
    std::make_tuple( text_codec::DONE, &*s.end(), &*s32.begin() ),
    codec.decode( &*s.end(), &*s.begin(), &*s32.begin(), &*s32.end(), true ) );

  EXPECT_EQ(
    std::make_tuple( text_codec::OUT_OF_SPACE, &*s.begin(), &*s32.end() ),
    codec.decode( &*s.begin(), &*s.end(), &*s32.end(), &*s32.begin(), true ) );

  EXPECT_EQ(
    std::make_tuple( text_codec::DONE, &*s32.end(), &*s.begin() ),
    codec.encode( &*s32.end(), &*s32.begin(), &*s.begin(), &*s.end() ) );

  EXPECT_EQ(
    std::make_tuple( text_codec::OUT_OF_SPACE, &*s32.begin(), &*s.end() ),
    codec.encode( &*s32.begin(), &*s32.end(), &*s.end(), &*s.begin() ) );
}

#endif
