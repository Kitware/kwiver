// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the UTF-8 text codec.

#include <vital/util/tests/test_text_codec.h>
#include <vital/util/text_codec_error_policies.h>
#include <vital/util/text_codec_utf_8.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( text_codec, utf_8 )
{
  text_codec_utf_8 codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_abort::instance() );
  codec.set_decode_error_policy(
    text_codec_decode_error_policy_abort::instance() );

  CALL_TEST( test_codec_invalid_ranges, codec );

  CALL_TEST( test_codec_round_trip, codec, "", U"" );
  CALL_TEST( test_codec_round_trip, codec, "Kitware", U"Kitware" );
  CALL_TEST( test_codec_round_trip, codec, "Kĩtwārę", U"Kĩtwārę" );
  CALL_TEST( test_codec_round_trip, codec, "Ḱḯṫẃḁṝḕ", U"Ḱḯṫẃḁṝḕ" );
  CALL_TEST( test_codec_round_trip, codec, "🁙🂽🫖🦋🛸", U"🁙🂽🫖🦋🛸" );

  CALL_TEST( test_codec_encode_abort, codec, U"\xFFFFFFFF", "" );
  CALL_TEST( test_codec_encode_abort, codec, U"A\xFFFFFFFF", "A" );
  CALL_TEST( test_codec_encode_abort, codec, U"A\xFFFFFFFF""B", "A" );
  CALL_TEST( test_codec_encode_abort, codec, U"\xFFFFFFFF""B", "" );

  CALL_TEST( test_codec_decode_abort, codec, "\xC0\x80", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\x80", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "A\x80", U"A" );
  CALL_TEST( test_codec_decode_abort, codec, "A\x80""B", U"A" );
  CALL_TEST( test_codec_decode_abort, codec, "\x80""B", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xC0\xFF", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xC0""B", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xBF", U"" );

  CALL_TEST( test_codec_encode_out_of_space, codec, U"AB", "A", 1, 1 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"A🛸", "A", 4, 1 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"Aḯ", "A", 3, 1 );

  CALL_TEST( test_codec_decode_out_of_space, codec, "A🛸", U"A", 1, 1 );
  CALL_TEST( test_codec_decode_out_of_space, codec, "🛸", U"", 0, 0 );
}

// ----------------------------------------------------------------------------
TEST ( text_codec, utf_8_long )
{
  text_codec_utf_8 codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_abort::instance() );
  codec.set_decode_error_policy(
    text_codec_decode_error_policy_abort::instance() );

  // 3-byte character used to ensure a character spans across buffers
  std::string s( BUFSIZ * 9, 'A' );
  for( size_t i = 0; i < BUFSIZ * 9; ++i )
  {
    s[ i ] = "ḯ"[ i % 3 ];
  }
  CALL_TEST( test_codec_round_trip, codec,
    s, std::u32string( BUFSIZ * 3, U'ḯ' ) );
}
