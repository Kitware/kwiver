// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the ASCII text codec.

#include <vital/util/tests/test_text_codec.h>
#include <vital/util/text_codec_ascii.h>
#include <vital/util/text_codec_error_policies.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( text_codec, ascii )
{
  text_codec_ascii codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_abort::instance() );
  codec.set_decode_error_policy(
    text_codec_decode_error_policy_abort::instance() );

  CALL_TEST( test_codec_invalid_ranges, codec );

  CALL_TEST( test_codec_round_trip, codec, "", U"" );
  CALL_TEST( test_codec_round_trip, codec,
    std::string{ "\0", 1 }, std::u32string{ U"\u0000", 1 } );
  CALL_TEST( test_codec_round_trip, codec, "\x7F", U"\u007F" );
  CALL_TEST( test_codec_round_trip, codec, "Kitware", U"Kitware" );

  CALL_TEST( test_codec_encode_abort, codec, U"\u0080", "" );
  CALL_TEST( test_codec_encode_abort, codec, U"\u00FF", "" );
  CALL_TEST( test_codec_encode_abort, codec, U"A\u00FF", "A" );
  CALL_TEST( test_codec_encode_abort, codec, U"A\u00FFB", "A" );
  CALL_TEST( test_codec_encode_abort, codec, U"\u00FFB", "" );

  CALL_TEST( test_codec_decode_abort, codec, "\x80", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xFF", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "A\xFF", U"A" );
  CALL_TEST( test_codec_decode_abort, codec, "A\xFF""B", U"A" );
  CALL_TEST( test_codec_decode_abort, codec, "\xFF""B", U"" );

  CALL_TEST( test_codec_encode_out_of_space, codec, U"A", "", 0, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"AA", "A", 1, 1 );

  CALL_TEST( test_codec_decode_out_of_space, codec, "A", U"", 0, 0 );
  CALL_TEST( test_codec_decode_out_of_space, codec, "AA", U"A", 1, 1 );
}

// ----------------------------------------------------------------------------
TEST ( text_codec, ascii_error_skip )
{
  text_codec_ascii codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_skip::instance() );
  codec.set_decode_error_policy(
    text_codec_decode_error_policy_skip::instance() );

  CALL_TEST( test_codec_encode, codec, U"\uFFFF", "" );
  CALL_TEST( test_codec_encode, codec, U"A\uFFFF", "A" );
  CALL_TEST( test_codec_encode, codec, U"\uFFFFB", "B" );
  CALL_TEST( test_codec_encode, codec, U"A\uFFFFB", "AB" );

  CALL_TEST( test_codec_decode, codec, "\xFF", U"" );
  CALL_TEST( test_codec_decode, codec, "A\xFF", U"A" );
  CALL_TEST( test_codec_decode, codec, "\xFF""B", U"B" );
  CALL_TEST( test_codec_decode, codec, "A\xFF""B", U"AB" );
}

// ----------------------------------------------------------------------------
TEST ( text_codec, ascii_error_substitute )
{
  text_codec_ascii codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_substitute::instance() );
  codec.set_decode_error_policy(
    text_codec_decode_error_policy_substitute::instance() );

  CALL_TEST( test_codec_encode, codec, U"\uFFFF", "\x1A" );
  CALL_TEST( test_codec_encode, codec, U"A\uFFFF", "A\x1A" );
  CALL_TEST( test_codec_encode, codec, U"\uFFFFB", "\x1A""B" );
  CALL_TEST( test_codec_encode, codec, U"A\uFFFFB", "A\x1A""B" );

  CALL_TEST( test_codec_decode, codec, "\xFF", U"\u001A" );
  CALL_TEST( test_codec_decode, codec, "A\xFF", U"A\u001A" );
  CALL_TEST( test_codec_decode, codec, "\xFF""B", U"\u001AB" );
  CALL_TEST( test_codec_decode, codec, "A\xFF""B", U"A\u001AB" );

  CALL_TEST( test_codec_encode_out_of_space, codec, U"\uFFFF", "", 0, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec,
    U"\uFFFF\uFFFF", "\x1A", 1, 1 );

  CALL_TEST( test_codec_decode_out_of_space, codec, "\xFF", U"", 0, 0 );
  CALL_TEST( test_codec_decode_out_of_space, codec,
    "\xFF\xFF", U"\u001A", 1, 1 );
}

// ----------------------------------------------------------------------------
TEST ( text_codec, ascii_error_unicode_escape )
{
  text_codec_ascii codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_unicode_escape::instance() );

  CALL_TEST( test_codec_encode, codec, U"\u0080", "\\u0080" );
  CALL_TEST( test_codec_encode, codec, U"\u00FF", "\\u00FF" );
  CALL_TEST( test_codec_encode, codec, U"\uFFFF", "\\uFFFF" );
  CALL_TEST( test_codec_encode, codec, U"A\uFFFFB", "A\\uFFFFB" );
  CALL_TEST( test_codec_encode, codec, U"\U00010000", "\\U00010000" );
  CALL_TEST( test_codec_encode, codec, U"A\U00010000B", "A\\U00010000B" );
  CALL_TEST( test_codec_encode, codec, U"\U0010FFFF", "\\U0010FFFF" );
  CALL_TEST( test_codec_encode, codec, U"\x00110000", "\\U00110000" );
  CALL_TEST( test_codec_encode, codec, U"\x89ABCDEF", "\\U89ABCDEF" );
  CALL_TEST( test_codec_encode, codec, U"\xFFFFFFFF", "\\UFFFFFFFF" );

  CALL_TEST( test_codec_encode_out_of_space, codec, U"\uFFFF", "", 5, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"\U0010FFFF", "", 9, 0 );
}

// ----------------------------------------------------------------------------
TEST ( text_codec, ascii_long )
{
  text_codec_ascii codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_abort::instance() );
  codec.set_decode_error_policy(
    text_codec_decode_error_policy_abort::instance() );

  CALL_TEST( test_codec_round_trip, codec,
     std::string( BUFSIZ * 3, 'A' ), std::u32string( BUFSIZ * 3, U'A' ) );
}
