// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test CSV input/output utilities.

#include <arrows/core/csv_io.h>

#include <tests/test_gtest.h>

#include <limits>
#include <sstream>

#include <cmath>

using namespace kwiver::arrows::core;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( csv_io, write_empty_lines )
{
  std::stringstream ss;
  {
    csv_writer writer( ss );
    writer << csv::endl << csv::endl;
  }
  EXPECT_EQ( ss.str(), "\n\n" );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, write_all_types )
{
  std::stringstream ss;
  {
    csv_writer writer( ss );
    writer
      << '\0'
      << true
      << "2"
      << std::string{ "3" }
      << uint8_t{ 4 }
      << uint16_t{ 5 }
      << uint32_t{ 6 }
      << uint64_t{ 7 }
      << int8_t{ 8 }
      << int16_t{ 9 }
      << int32_t{ 10 }
      << int64_t{ 11 }
      << 12.12f
      << 13.13;
  }

  EXPECT_EQ( ss.str(), "0,true,2,3,4,5,6,7,8,9,10,11,12.12,13.13\n" );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, write_quote )
{
  std::stringstream ss;
  {
    csv_writer writer( ss );
    writer
      << "a comma:," << "an endl: \n" << "" << "# not a comment"
      << "a quote: \"" << "a double quote: \"\"";
  }

  EXPECT_EQ(
    ss.str(),
    "\"a comma:,\",\"an endl: \n\",,\"# not a comment\","
    "\"a quote: \"\"\",\"a double quote: \"\"\"\"\"\n" );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, write_field )
{
  std::stringstream ss;
  {
    csv_writer writer( ss );
    writer
      << csv::begf << csv::endf << csv::endl
      << "before"
      << csv::begf << true << ", that " << 7 << " equals " << 7 << csv::endf
      << "after"
      << csv::endl;

    // Check error cases
    EXPECT_THROW( writer << csv::endf, std::invalid_argument );
    writer << csv::begf;
    EXPECT_THROW( writer << csv::begf, std::invalid_argument );
    EXPECT_THROW( writer << csv::endl, std::invalid_argument );
    EXPECT_THROW( writer << csv::comment, std::invalid_argument );
  }

  EXPECT_EQ( ss.str(), "\nbefore,\"true, that 7 equals 7\",after\n" );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, write_comment )
{
  std::stringstream ss;
  {
    csv_writer writer( ss );
    writer
      << "regular" << "line" << csv::endl
      << csv::comment << "comment" << "line" << csv::endl
      << "regular" << "line";

    // Check error cases
    EXPECT_THROW( writer << csv::comment, std::invalid_argument );
    writer << csv::endl << csv::begf;
    EXPECT_THROW( writer << csv::comment, std::invalid_argument );
  }

  EXPECT_EQ( ss.str(), "regular,line\n#comment,line\nregular,line\n" );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, write_different_special_chars )
{
  std::stringstream ss;
  {
    csv_writer writer( ss, ';', '\'', '\\' );
    writer
      << "comma," << "semi;" << "quote'" << "backslashquote\\'" << csv::endl;
  }

  EXPECT_EQ( ss.str(), "comma,;'semi;';'quote\\'';'backslashquote\\\\\\''\n" );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_empty )
{
  std::stringstream ss{ "" };
  csv_reader reader( ss );
  EXPECT_TRUE( reader.is_at_eol() );
  EXPECT_TRUE( reader.is_at_eof() );
  EXPECT_THROW( reader.read< std::string >(), std::invalid_argument );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_empty_lines )
{
  std::stringstream ss{ "\n\n" };
  csv_reader reader( ss );
  EXPECT_TRUE( reader.is_at_eol() );
  EXPECT_TRUE( reader.is_at_eof() );
  EXPECT_THROW( reader.read< std::string >(), std::invalid_argument );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_mixed_empty_lines )
{
  std::stringstream ss{ "\n\nfield1\nfield2\n\nfield3\n\n\n" };
  csv_reader reader( ss );
  EXPECT_TRUE( reader.is_at_field() );
  EXPECT_EQ( "field1", reader.read< std::string >() );
  EXPECT_TRUE( reader.is_at_eol() );
  EXPECT_FALSE( reader.is_at_eof() );
  reader.next_line();

  EXPECT_TRUE( reader.is_at_field() );
  EXPECT_EQ( "field2", reader.read< std::string >() );
  EXPECT_TRUE( reader.is_at_eol() );
  EXPECT_FALSE( reader.is_at_eof() );
  reader.next_line();

  EXPECT_TRUE( reader.is_at_field() );
  EXPECT_EQ( "field3", reader.read< std::string >() );
  EXPECT_TRUE( reader.is_at_eol() );
  EXPECT_TRUE( reader.is_at_eof() );
  EXPECT_FALSE( reader.is_at_field() );
  EXPECT_THROW( reader.next_line(), std::invalid_argument );
  EXPECT_THROW( reader.skip_line(), std::invalid_argument );
  EXPECT_THROW( reader.read< std::string >(), std::invalid_argument );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_all_types )
{
  std::stringstream ss{
    "0,1,2,3,4,5,6,7,8,9,10,11,12\n"
    "\"0\",\"1\",\"2\",\"3\",\"4\",\"5\",\"6\","
    "\"7\",\"8\",\"9\",\"10\",\"11\",\"12\"\n" };
  csv_reader reader( ss );
  EXPECT_FALSE( reader.is_at_eol() );
  EXPECT_FALSE( reader.is_at_eof() );

  for( size_t i = 0; i < 2; ++i )
  {
    EXPECT_EQ( '\0', reader.read< char >() );
    EXPECT_EQ( true, reader.read< bool >() );
    EXPECT_EQ( "2", reader.read< std::string >() );
    EXPECT_EQ( 3, reader.read< uint8_t >() );
    EXPECT_EQ( 4, reader.read< uint16_t >() );
    EXPECT_EQ( 5, reader.read< uint32_t >() );
    EXPECT_EQ( 6, reader.read< uint64_t >() );
    EXPECT_EQ( 7, reader.read< int8_t >() );
    EXPECT_EQ( 8, reader.read< int16_t >() );
    EXPECT_EQ( 9, reader.read< int32_t >() );
    EXPECT_EQ( 10, reader.read< int64_t >() );
    EXPECT_EQ( 11.0f, reader.read< float >() );
    EXPECT_EQ( 12.0, reader.read< double >() );
    EXPECT_TRUE( reader.is_at_eol() );
    EXPECT_THROW( reader.read< std::string >(), std::invalid_argument );
    if( i == 0 )
    {
      reader.next_line();
    }
    else
    {
      EXPECT_TRUE( reader.is_at_eof() );
    }
  }
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_bool )
{
  for( auto const s : { "0", "false" } )
  {
    std::stringstream ss{ s };
    csv_reader reader( ss );
    EXPECT_EQ( false, reader.read< bool >() );
  }

  for( auto const s : { "1", "true" } )
  {
    std::stringstream ss{ s };
    csv_reader reader( ss );
    EXPECT_EQ( true, reader.read< bool >() );
  }

  for( auto const s : { "\"\"", "Kitware", "7" } )
  {
    std::stringstream ss{ s };
    csv_reader reader{ ss };
    EXPECT_THROW( reader.read< bool >(), csv_reader::parse_error );
  }
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_int )
{
  std::stringstream ss{
    " 1,1 ,1,128,-129,2,256,-1,\"3\",32768,-32769,4,65536,-1,5,4294967296,6,"
    "99999999999999999999,\"7\"" };
  csv_reader reader( ss );
  EXPECT_THROW( reader.read< int >(), csv_reader::parse_error );
  EXPECT_THROW( reader.read< int >(), csv_reader::parse_error );
  EXPECT_EQ( 1, reader.read< int8_t >() );
  EXPECT_THROW( reader.read< int8_t >(), csv_reader::parse_error );
  EXPECT_THROW( reader.read< int8_t >(), csv_reader::parse_error );
  EXPECT_EQ( 2, reader.read< uint8_t >() );
  EXPECT_THROW( reader.read< uint8_t >(), csv_reader::parse_error );
  EXPECT_THROW( reader.read< uint8_t >(), csv_reader::parse_error );
  EXPECT_EQ( 3, reader.read< int16_t >() );
  EXPECT_THROW( reader.read< int16_t >(), csv_reader::parse_error );
  EXPECT_THROW( reader.read< int16_t >(), csv_reader::parse_error );
  EXPECT_EQ( 4, reader.read< uint16_t >() );
  EXPECT_THROW( reader.read< uint16_t >(), csv_reader::parse_error );
  EXPECT_THROW( reader.read< uint16_t >(), csv_reader::parse_error );
  EXPECT_EQ( 5, reader.read< int32_t >() );
  EXPECT_THROW( reader.read< uint32_t >(), csv_reader::parse_error );
  EXPECT_EQ( 6, reader.read< int64_t >() );
  EXPECT_THROW( reader.read< uint64_t >(), csv_reader::parse_error );
  EXPECT_EQ( 7, reader.read< int64_t >() );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_float )
{
  std::stringstream ss{
    " 1,1 ,\"1\",1.,1.0,-1.0,3.6e10\n"
    "inf,INF,-inf,-INF,nan,NAN,\"-nan\",-NAN,nanana" };
  csv_reader reader( ss );
  EXPECT_THROW( reader.read< float >(), csv_reader::parse_error );
  EXPECT_THROW( reader.read< double >(), csv_reader::parse_error );
  EXPECT_EQ( 1.0f, reader.read< float >() );
  EXPECT_EQ( 1.0f, reader.read< float >() );
  EXPECT_EQ( 1.0, reader.read< double >() );
  EXPECT_EQ( -1.0f, reader.read< float >() );
  EXPECT_EQ( 3.6e10, reader.read< double >() );
  EXPECT_NO_THROW( reader.next_line() );
  EXPECT_EQ( std::numeric_limits< float >::infinity(),
             reader.read< float >() );
  EXPECT_EQ( std::numeric_limits< double >::infinity(),
             reader.read< double >() );
  EXPECT_EQ( -std::numeric_limits< float >::infinity(),
             reader.read< float >() );
  EXPECT_EQ( -std::numeric_limits< double >::infinity(),
             reader.read< double >() );
  EXPECT_TRUE( std::isnan( reader.read< float >() ) );
  EXPECT_TRUE( std::isnan( reader.read< double >() ) );
  EXPECT_TRUE( std::isnan( reader.read< float >() ) );
  EXPECT_TRUE( std::isnan( reader.read< double >() ) );
  EXPECT_THROW( reader.read< float >(), csv_reader::parse_error );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_string )
{
  std::stringstream ss{
    ",\"\",s t r i n g,\",\",quote\"quote,\"\"\"\",\"\n\n\",\n" };
  csv_reader reader( ss );
  EXPECT_EQ( "", reader.read< std::string >() );
  EXPECT_EQ( "", reader.read< std::string >() );
  EXPECT_EQ( "s t r i n g", reader.read< std::string >() );
  EXPECT_EQ( ",", reader.read< std::string >() );
  EXPECT_EQ( "quote\"quote", reader.read< std::string >() ); // Should warn
  EXPECT_EQ( "\"", reader.read< std::string >() );
  EXPECT_EQ( "\n\n", reader.read< std::string >() );
  EXPECT_FALSE( reader.is_at_eol() );
  EXPECT_FALSE( reader.is_at_eof() );
  EXPECT_EQ( "", reader.read< std::string >() );
  EXPECT_TRUE( reader.is_at_eol() );
  EXPECT_TRUE( reader.is_at_eof() );
  EXPECT_THROW( reader.read< std::string >(), std::invalid_argument );
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_string_invalid )
{
  {
    std::stringstream ss{ "\"" };
    csv_reader reader( ss );
    EXPECT_EQ( "", reader.read< std::string >() ); // Should warn
  }
  {
    std::stringstream ss{ "\"\"\"" };
    csv_reader reader( ss );
    EXPECT_EQ( "\"", reader.read< std::string >() ); // Should warn
  }
}

// ----------------------------------------------------------------------------
TEST ( csv_io, read_comment )
{
  std::stringstream ss{
    "#this,is,a,comment\n"
    "not # a comment\n"
    "not,# a comment either\n"
    "\n"
    "#another,comment\n"
    "data\n"
    "#ending,comment\n" };
  csv_reader reader( ss );
  EXPECT_NO_THROW( reader.read< csv::comment_t >() );
  EXPECT_TRUE( reader.is_at_field() );
  EXPECT_EQ( "this", reader.read< std::string >() );
  EXPECT_TRUE( reader.is_at_field() );
  reader.skip_field();
  EXPECT_TRUE( reader.is_at_field() );
  reader.skip_field();
  EXPECT_TRUE( reader.is_at_field() );
  EXPECT_EQ( "comment", reader.read< std::string >() );
  EXPECT_FALSE( reader.is_at_field() );
  EXPECT_TRUE( reader.is_at_eol() );
  reader.next_line();
  EXPECT_EQ( "not # a comment", reader.read< std::string >() );
  reader.next_line();
  EXPECT_EQ( "not", reader.read< std::string >() );
  EXPECT_EQ( "# a comment either", reader.read< std::string >() );
  reader.next_line();
  EXPECT_EQ( "data", reader.read< std::string >() );
  EXPECT_TRUE( reader.is_at_eol() );
  reader.next_line();
  EXPECT_TRUE( reader.is_at_comment() );
  EXPECT_FALSE( reader.is_at_field() );
  reader.skip_line();
  EXPECT_TRUE( reader.is_at_eof() );
}
