// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/types/metadata.h>
#include <vital/types/metadata_traits.h>

#include <gtest/gtest.h>

#include <memory>

#include <cstdint>

using namespace ::kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST( metadata, typed_metadata )
{
  // create item
  auto tmds = typed_metadata< VITAL_META_METADATA_ORIGIN, std::string >{
    "item data", std::string{ "origin" } };
  auto tmdd = typed_metadata< VITAL_META_PLATFORM_HEADING_ANGLE, double >{
    std::string{ "test double item" }, 3.14159 };
  auto tmdi = typed_metadata< VITAL_META_UNIX_TIMESTAMP, uint64_t >{
    std::string{ "test uint item" }, uint64_t{ 314159 } };

  // test API
  EXPECT_TRUE( tmds.has_string() );
  EXPECT_FALSE( tmds.has_double() );
  EXPECT_FALSE( tmds.has_uint64() );
  EXPECT_EQ( "origin", tmds.as_string() );

  EXPECT_FALSE( tmdd.has_string() );
  EXPECT_TRUE( tmdd.has_double() );
  EXPECT_FALSE( tmdd.has_uint64() );
  EXPECT_FLOAT_EQ( 3.14159, tmdd.as_double() );
  EXPECT_EQ( "3.14159", tmdd.as_string() );

  EXPECT_FALSE( tmdi.has_string() );
  EXPECT_FALSE( tmdi.has_double() );
  EXPECT_TRUE( tmdi.has_uint64() );
  EXPECT_EQ( 314159, tmdi.as_uint64() );
  EXPECT_EQ( "314159", tmdi.as_string() );
}

// ----------------------------------------------------------------------------
TEST( metadata, add_metadata )
{
  // create item
  using rmdi_t = typed_metadata< VITAL_META_UNIX_TIMESTAMP, uint64_t >;
  auto rmdi = rmdi_t{ "test uint item", uint64_t{ 314159 } };

  using umdd_t = typed_metadata< VITAL_META_PLATFORM_HEADING_ANGLE, double >;
  auto umdd = std::unique_ptr< umdd_t >{
    new umdd_t{ "test double item" , 3.14159 } };

  metadata meta_collection;

  meta_collection.add< VITAL_META_METADATA_ORIGIN >( "item data" );
  meta_collection.add( std::move( umdd ) );
  meta_collection.add_copy( rmdi );

  {
    EXPECT_TRUE( meta_collection.has( VITAL_META_METADATA_ORIGIN ) );

    auto const& md = meta_collection.find( VITAL_META_METADATA_ORIGIN );
    EXPECT_TRUE( md.has_string() );
    EXPECT_EQ( "item data",  md.as_string() );
  }

  {
    EXPECT_TRUE( meta_collection.has( VITAL_META_PLATFORM_HEADING_ANGLE ) );

    auto const& md = meta_collection.find( VITAL_META_PLATFORM_HEADING_ANGLE );
    EXPECT_TRUE( md.has_double() );
    EXPECT_FALSE( md.has_string() );
    EXPECT_FLOAT_EQ( 3.14159, md.as_double() );
    EXPECT_EQ( "3.14159", md.as_string() );
  }

  {
    EXPECT_TRUE( meta_collection.has( VITAL_META_UNIX_TIMESTAMP ) );

    auto const& md = meta_collection.find( VITAL_META_UNIX_TIMESTAMP );
    EXPECT_FALSE( md.has_string() );
    EXPECT_FALSE( md.has_double() );
    EXPECT_TRUE( md.has_uint64() );
    EXPECT_EQ( 314159, md.as_uint64() );
    EXPECT_EQ( "314159", md.as_string() );
  }

  EXPECT_EQ( 3, meta_collection.size() );
  EXPECT_FALSE( meta_collection.empty() );
}
