// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Utility function templates for testing read/write for KLV data
/// formats.

#include <arrows/klv/klv_data_format.h>

#include <vital/util/demangle.h>

#include <tests/test_gtest.h>

// ----------------------------------------------------------------------------
using namespace kwiver::arrows::klv;
namespace kv = kwiver::vital;

// ----------------------------------------------------------------------------
template < class Format >
void
test_read_format( klv_value const& expected_result, klv_bytes_t const& bytes )
{
  Format const format;
  auto it = bytes.cbegin();
  auto const result = format.read( it, bytes.size() );
  ASSERT_EQ( bytes.cend(), it );
  ASSERT_EQ( expected_result.type(), result.type() )
        << "\n  --type difference--\n  "
        << kv::demangle( expected_result.type().name() )
        << "\n  --versus--\n  "
        << kv::demangle( result.type().name() );
  EXPECT_EQ( expected_result, result )
        << "\n  --value difference--\n  "
        << format.to_string( expected_result )
        << "\n  --versus--\n  "
        << format.to_string( result );
}

// ----------------------------------------------------------------------------
template < class Format >
void
test_write_format( klv_value const& value )
{
  Format const format;
  klv_bytes_t bytes( format.length_of( value ) );
  auto write_it = bytes.begin();
  format.write( value, write_it, bytes.size() );
  ASSERT_EQ( bytes.end(), write_it );

  auto read_it = bytes.cbegin();
  auto const result = format.read( read_it, bytes.size() );
  ASSERT_EQ( value.type(), result.type() )
        << "\n  --type difference--\n  "
        << kv::demangle( value.type().name() )
        << "\n  --versus--\n  "
        << kv::demangle( result.type().name() );
  EXPECT_EQ( value, result )
        << "\n  --value difference--\n  "
        << format.to_string( value )
        << format.to_string( result );
}

// ----------------------------------------------------------------------------
template < class Format >
void
test_read_write_format( klv_value const& expected_result,
                        klv_bytes_t const& bytes )
{
  Format const format;
  auto it = bytes.cbegin();
  auto result = format.read( it, bytes.size() );
  ASSERT_EQ( bytes.cend(), it );
  ASSERT_EQ( expected_result.type(), result.type() )
        << "\n  --type difference--\n  "
        << kv::demangle( expected_result.type().name() )
        << "\n  --versus--\n  "
        << kv::demangle( result.type().name() );
  EXPECT_EQ( expected_result, result )
        << "\n  --value difference--\n  "
        << format.to_string( expected_result )
        << "\n  --versus--\n  "
        << format.to_string( result );

  klv_bytes_t buffer( format.length_of( result ) );
  auto write_it = buffer.begin();
  format.write( result, write_it, buffer.size() );
  ASSERT_EQ( buffer.end(), write_it );

  auto read_it = buffer.cbegin();
  result = format.read( read_it, buffer.size() );
  ASSERT_EQ( expected_result.type(), result.type() )
        << "\n  --type difference--\n  "
        << kv::demangle( expected_result.type().name() )
        << "\n  --versus--\n  "
        << kv::demangle( result.type().name() );
  EXPECT_EQ( expected_result, result )
        << "\n  --value difference--\n  "
        << format.to_string( expected_result )
        << "\n  --versus--\n  "
        << format.to_string( result );
}
