// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV blob functions.

#include <arrows/klv/klv_blob.h>

#include <tests/test_gtest.h>

#include <vector>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

using namespace kwiver::arrows::klv;

// ----------------------------------------------------------------------------
void
test_blob_read( klv_bytes_t const& data )
{
  auto it = &*data.cbegin();
  EXPECT_EQ( data, *klv_read_blob( it, data.size() ) );
  EXPECT_EQ( &*data.cend(), it );
}

// ----------------------------------------------------------------------------
TEST( klv, blob_read )
{
  CALL_TEST( test_blob_read, {} );
  CALL_TEST( test_blob_read, { 0x00 } );
  CALL_TEST( test_blob_read, { 0xFF, 0xFF } );
  CALL_TEST( test_blob_read,
             { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00 } );
}

// ----------------------------------------------------------------------------
void
test_blob_write( klv_bytes_t const& data )
{
  klv_bytes_t buffer( data.size(), 0xba );
  auto it = &*buffer.begin();
  klv_write_blob( data, it, buffer.size() );
  EXPECT_EQ( &*buffer.end(), it );
  auto cit = &*buffer.cbegin();
  EXPECT_EQ( data, *klv_read_blob( cit, buffer.size() ) );
}

// ----------------------------------------------------------------------------
TEST( klv, blob_write )
{
  CALL_TEST( test_blob_read, {} );
  CALL_TEST( test_blob_read, { 0x00 } );
  CALL_TEST( test_blob_read, { 0xFF, 0xFF } );
  CALL_TEST( test_blob_read,
             { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00 } );
}

// ----------------------------------------------------------------------------
void
test_blob_length( klv_bytes_t const& data )
{
  EXPECT_EQ( data.size(), klv_blob_length( data ) );
}

// ----------------------------------------------------------------------------
TEST( klv, blob_length )
{
  CALL_TEST( test_blob_length, {} );
  CALL_TEST( test_blob_length, { 0x00 } );
  CALL_TEST( test_blob_length, { 0xBA, 0xDA } );
  CALL_TEST( test_blob_length,
             { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00 } );
}
