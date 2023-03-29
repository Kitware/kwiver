// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the text transcoding utilities.

#include <vital/util/tests/test_text_codec.h>
#include <vital/util/text_codec_error_policies.h>
#include <vital/util/text_codec_ascii.h>
#include <vital/util/text_codec_utf_8.h>
#include <vital/util/text_codec_utf_16.h>
#include <vital/util/text_codec_transcode.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
void
test_transcode(
  text_codec const& src_codec, text_codec const& dst_codec,
  std::string const& src, std::string const& dst )
{
  auto const size = text_codec_transcoded_size( src_codec, dst_codec, src );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, dst.size() ), size );

  auto const src_to_dst = text_codec_transcode( src_codec, dst_codec, src );
  EXPECT_EQ( std::make_tuple( text_codec::DONE, dst ), src_to_dst );

  text_transcoder transcoder( src_codec, dst_codec );
  std::string dst_buffer( dst.size(), '\0' );
  auto const src_to_dst_buffered =
    transcoder.transcode(
      &*src.begin(), &*src.end(),
      &*dst_buffer.begin(), &*dst_buffer.end(), true );
  EXPECT_EQ(
    std::make_tuple( text_codec::DONE, &*src.end(), &*dst_buffer.end() ),
    src_to_dst_buffered );
}

// ----------------------------------------------------------------------------
void
test_transcode_round_trip(
  text_codec const& src_codec, text_codec const& dst_codec,
  std::string const& src, std::string const& dst )
{
  CALL_TEST( test_transcode, src_codec, dst_codec, src, dst );
  CALL_TEST( test_transcode, dst_codec, src_codec, dst, src );
}

// ----------------------------------------------------------------------------
TEST ( text_codec, transcode )
{
  text_codec_ascii ascii;
  text_codec_utf_8 utf_8;
  text_codec_utf_16_be utf_16;

  CALL_TEST( test_transcode_round_trip, ascii, utf_8, "", "" );
  CALL_TEST( test_transcode_round_trip, ascii, utf_8, "Kitware", "Kitware" );
  CALL_TEST( test_transcode_round_trip, utf_8, utf_8, "", "" );
  CALL_TEST( test_transcode_round_trip, utf_8, utf_8,
    "🁙🂽🫖🦋🛸", "🁙🂽🫖🦋🛸" );
  CALL_TEST( test_transcode_round_trip, utf_8, utf_16,
    "🁙🂽🫖🦋🛸",
    "\xD8\x3C\xDC\x59"
    "\xD8\x3C\xDC\xBD"
    "\xD8\x3E\xDE\xD6"
    "\xD8\x3E\xDD\x8B"
    "\xD8\x3D\xDE\xF8" );

  CALL_TEST( test_transcode, ascii, ascii, "\xFF", "\x1A" );
  CALL_TEST( test_transcode, ascii, utf_8, "\xFF", "\x1A" );
  CALL_TEST( test_transcode, utf_8, ascii, "\xFF", "\x1A" );
  CALL_TEST( test_transcode, utf_8, ascii,
    "Ḱḯṫẃḁṝḕ", "\x1A\x1A\x1A\x1A\x1A\x1A\x1A" );
  CALL_TEST( test_transcode, utf_8, ascii, "ABC\x80""DEF", "ABC\x1A""DEF" );
  CALL_TEST( test_transcode, utf_8, utf_8, "\x80", "\uFFFD" );
  CALL_TEST( test_transcode, utf_8, utf_16,
    "ABC\x80""DEF", std::string{ "\0A\0B\0C\xFF\xFD\0D\0E\0F", 14 } );
}

// ----------------------------------------------------------------------------
TEST ( text_codec, transcode_long )
{
  text_codec_utf_8 utf_8;
  text_codec_utf_16_be utf_16;

  // 3-byte character used to ensure a character spans across buffers
  std::string src( BUFSIZ * 9, 'A' );
  for( size_t i = 0; i < src.size(); ++i )
  {
    src[ i ] = "ḯ"[ i % 3 ];
  }
  std::string dst( BUFSIZ * 6, 'A' );
  for( size_t i = 0; i < dst.size(); ++i )
  {
    dst[ i ] = "\x1E\x2F"[ i % 2 ];
  }
  CALL_TEST( test_transcode_round_trip, utf_8, utf_8, src, src );
  CALL_TEST( test_transcode_round_trip, utf_8, utf_16, src, dst );
  CALL_TEST( test_transcode_round_trip, utf_16, utf_16, dst, dst );
}
