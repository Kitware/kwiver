// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the UTF-16 text codecs.

#include <vital/util/tests/test_text_codec.h>
#include <vital/util/text_codec_utf_16.h>
#include <vital/util/text_codec_error_policies.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( text_codec, utf_16_be )
{
  text_codec_utf_16_be codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_abort::instance() );
  codec.set_decode_error_policy(
    text_codec_decode_error_policy_abort::instance() );

  CALL_TEST( test_codec_invalid_ranges, codec );

  CALL_TEST( test_codec_round_trip, codec, "", U"" );
  CALL_TEST( test_codec_round_trip, codec,
    std::string( "\0K\0i\0t\0w\0a\0r\0e", 14 ), U"Kitware" );
  CALL_TEST( test_codec_round_trip, codec,
    std::string( "\0K\x01\x29\0t\0w\x01\x01\0r\x01\x19", 14 ), U"Kĩtwārę" );
  CALL_TEST( test_codec_round_trip, codec,
    "\x1E\x30\x1E\x2F\x1E\x6B\x1E\x83\x1E\x01\x1E\x5D\x1E\x15", U"Ḱḯṫẃḁṝḕ" );
  CALL_TEST( test_codec_round_trip, codec,
    "\xD8\x3C\xDC\x59""\xD8\x3C\xDC\xBD""\xD8\x3E\xDE\xD6"
    "\xD8\x3E\xDD\x8B""\xD8\x3D\xDE\xF8", U"🁙🂽🫖🦋🛸" );

  CALL_TEST( test_codec_encode_abort, codec, U"\xFFFFFFFF", "" );
  CALL_TEST( test_codec_encode_abort, codec, U"\xD800", "" );
  CALL_TEST( test_codec_encode_abort, codec, U"\xDC00", "" );
  CALL_TEST( test_codec_encode_abort, codec,
    U"A\xFFFFFFFF", std::string( "\0A", 2 ) );
  CALL_TEST( test_codec_encode_abort, codec,
    U"A\xD800", std::string( "\0A", 2 ) );
  CALL_TEST( test_codec_encode_abort, codec,
    U"A\xDC00", std::string( "\0A", 2 ) );

  CALL_TEST( test_codec_decode_abort, codec, "\xD8\x3C\xDC", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xD8\x3C", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xD8\x3C\0B", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xD8", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xD8\x3D\xD8\xF8", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xD8\x3D\xD8", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xDC\x3D\xD8\xF8", U"" );

  CALL_TEST( test_codec_encode_out_of_space, codec,
    U"AB", std::string( "\0A", 2 ), 2, 1 );
  CALL_TEST( test_codec_encode_out_of_space, codec,
    U"AB", std::string( "\0A", 2 ), 3, 1 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"🛸", "", 3, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"🛸", "", 2, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"🛸", "", 1, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"🛸", "", 0, 0 );

  CALL_TEST( test_codec_decode_out_of_space, codec,
    std::string( "\0A\xD8\x3D\xDE\xF8", 6 ), U"A", 1, 2 );
}

// ----------------------------------------------------------------------------
TEST ( text_codec, utf_16_le )
{
  text_codec_utf_16_le codec;
  codec.set_encode_error_policy(
    text_codec_encode_error_policy_abort::instance() );
  codec.set_decode_error_policy(
    text_codec_decode_error_policy_abort::instance() );

  CALL_TEST( test_codec_invalid_ranges, codec );

  CALL_TEST( test_codec_round_trip, codec, "", U"" );
  CALL_TEST( test_codec_round_trip, codec,
    std::string( "K\0i\0t\0w\0a\0r\0e\0", 14 ), U"Kitware" );
  CALL_TEST( test_codec_round_trip, codec,
    std::string( "K\0\x29\x01t\0w\0\x01\x01r\0\x19\x01", 14 ), U"Kĩtwārę" );
  CALL_TEST( test_codec_round_trip, codec,
    "\x30\x1E\x2F\x1E\x6B\x1E\x83\x1E\x01\x1E\x5D\x1E\x15\x1E", U"Ḱḯṫẃḁṝḕ" );
  CALL_TEST( test_codec_round_trip, codec,
    "\x3C\xD8\x59\xDC""\x3C\xD8\xBD\xDC""\x3E\xD8\xD6\xDE"
    "\x3E\xD8\x8B\xDD""\x3D\xD8\xF8\xDE", U"🁙🂽🫖🦋🛸" );

  CALL_TEST( test_codec_encode_abort, codec, U"\xFFFFFFFF", "" );
  CALL_TEST( test_codec_encode_abort, codec, U"\xD800", "" );
  CALL_TEST( test_codec_encode_abort, codec, U"\xDC00", "" );
  CALL_TEST( test_codec_encode_abort, codec,
    U"A\xFFFFFFFF", std::string( "A\0", 2 ) );
  CALL_TEST( test_codec_encode_abort, codec,
    U"A\xD800", std::string( "A\0", 2 ) );
  CALL_TEST( test_codec_encode_abort, codec,
    U"A\xDC00", std::string( "A\0", 2 ) );

  CALL_TEST( test_codec_decode_abort, codec, "\x3C\xD8\xDC", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\x3C\xD8", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\x3C\xD8""B\0", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\xD8", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\x3D\xD8\xF8\xD8", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\x3D\xD8\xD8", U"" );
  CALL_TEST( test_codec_decode_abort, codec, "\x3D\xDC\xF8\xD8", U"" );

  CALL_TEST( test_codec_encode_out_of_space, codec,
    U"AB", std::string( "A\0", 2 ), 2, 1 );
  CALL_TEST( test_codec_encode_out_of_space, codec,
    U"AB", std::string( "A\0", 2 ), 3, 1 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"🛸", "", 3, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"🛸", "", 2, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"🛸", "", 1, 0 );
  CALL_TEST( test_codec_encode_out_of_space, codec, U"🛸", "", 0, 0 );

  CALL_TEST( test_codec_decode_out_of_space, codec,
    std::string( "A\0\x3D\xD8\xF8\xDE", 6 ), U"A", 1, 2 );
}
