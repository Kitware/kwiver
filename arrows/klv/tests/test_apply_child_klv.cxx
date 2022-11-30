// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the apply_child_klv filter.

#include "data_format.h"

#include <vital/plugin_loader/plugin_manager.h>

#include <arrows/klv/apply_child_klv.h>
#include <arrows/klv/klv_metadata.h>
#include <arrows/klv/klv_0601.h>
#include <arrows/klv/klv_1607.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();
  return RUN_ALL_TESTS();
}

namespace kv = kwiver::vital;
using namespace kwiver::arrows::klv;

// ----------------------------------------------------------------------------
// Ensure we can create the filter with the factory method.
TEST ( apply_child_klv, create )
{
  EXPECT_NE( nullptr, kv::algo::metadata_filter::create( "apply_child_klv" ) );
}

// ----------------------------------------------------------------------------
// No metadata given.
TEST ( apply_child_klv, empty )
{
  apply_child_klv filter;
  kv::metadata_vector input;
  auto const output = filter.filter( input, nullptr );
  EXPECT_EQ( input, output );
}

// ----------------------------------------------------------------------------
// Null metadata pointer.
TEST ( apply_child_klv, null_metadata_sptr )
{
  apply_child_klv filter;
  kv::metadata_vector input{ nullptr };
  auto const output = filter.filter( input, nullptr );
  EXPECT_EQ( input, output );
}

// ----------------------------------------------------------------------------
// Metadata objects with no KLV attached.
TEST ( apply_child_klv, non_klv_metadata_sptr )
{
  apply_child_klv filter;
  kv::metadata_vector input{
    std::make_shared< kv::metadata >(),
    std::make_shared< kv::metadata >(),
  };
  input[ 0 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 0 );
  input[ 1 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 1 );
  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 2, output.size() );
  EXPECT_EQ(
    0, output.at( 0 )->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
  EXPECT_EQ(
    1, output.at( 1 )->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
}

// ----------------------------------------------------------------------------
// Metadata object with empty KLV attached.
TEST ( apply_child_klv, empty_klv )
{
  apply_child_klv filter;
  auto const klv_md = std::make_shared< klv_metadata >();
  kv::metadata_vector input{ klv_md };
  klv_md->set_klv( {} );
  klv_md->add< kv::VITAL_META_UNIX_TIMESTAMP >( 42 );
  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 1, output.size() );
  auto const output_klv =
    dynamic_cast< klv_metadata* >( output.at( 0 ).get() );
  ASSERT_NE( nullptr, output_klv );
  EXPECT_TRUE( output_klv->klv().empty() );
  EXPECT_EQ(
    42, output_klv->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
}

// ----------------------------------------------------------------------------
// KLV with no child sets.
TEST ( apply_child_klv, no_children )
{
  apply_child_klv filter;
  auto const klv_md = std::make_shared< klv_metadata >();
  kv::metadata_vector input{ klv_md };
  klv_md->set_klv( {
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } } } );
  klv_md->add< kv::VITAL_META_UNIX_TIMESTAMP >( 42 );

  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 1, output.size() );
  auto const output_klv =
    dynamic_cast< klv_metadata* >( output.at( 0 ).get() );
  ASSERT_NE( nullptr, output_klv );
  ASSERT_EQ( 1, output_klv->klv().size() );
  auto const& output_set =
    output_klv->klv().at( 0 ).value.get< klv_local_set >();
  EXPECT_EQ(
    42, output_set.at( KLV_0601_PRECISION_TIMESTAMP ).get< uint64_t >() );
  EXPECT_EQ(
    42, output_klv->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
}

// ----------------------------------------------------------------------------
// Nested amend sets.
TEST ( apply_child_klv, amend_only )
{
  apply_child_klv filter;
  auto const klv_md = std::make_shared< klv_metadata >();
  kv::metadata_vector input{ klv_md };

  klv_md->set_klv( {
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } },
      { KLV_0601_AMEND_LOCAL_SET, klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 43 } },
        { KLV_0601_MISSION_ID, std::string{ "ID" } },
        { KLV_0601_AMEND_LOCAL_SET, klv_local_set{
          { KLV_0601_MISSION_ID, std::string{ "BETTER_ID" } },
          { KLV_0601_PLATFORM_DESIGNATION, {} } } } } } } } } );
  klv_md->add< kv::VITAL_META_UNIX_TIMESTAMP >( 42 );

  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 1, output.size() );
  auto const output_klv =
    dynamic_cast< klv_metadata* >( output.at( 0 ).get() );
  ASSERT_NE( nullptr, output_klv );
  ASSERT_EQ( 1, output_klv->klv().size() );
  EXPECT_EQ(
    42, output_klv->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
  auto const& output_set =
    output_klv->klv().at( 0 ).value.get< klv_local_set >();

  std::vector< klv_packet > expected_klv = {
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 43 } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } },
      { KLV_0601_MISSION_ID, std::string{ "BETTER_ID" } },
      { KLV_0601_PLATFORM_DESIGNATION, {} } } } };

  EXPECT_EQ(
    std::multiset< klv_packet >( expected_klv.begin(), expected_klv.end() ),
    std::multiset< klv_packet >(
      output_klv->klv().begin(), output_klv->klv().end() ) );
}

// ----------------------------------------------------------------------------
// Sibling amend sets - minimal defined behavior for now except not to crash.
TEST ( apply_child_klv, sibling_amend )
{
  apply_child_klv filter;
  auto const klv_md = std::make_shared< klv_metadata >();
  kv::metadata_vector input{ klv_md };

  klv_md->set_klv( {
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_MISSION_ID, std::string{ "ID" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } },
      { KLV_0601_AMEND_LOCAL_SET, klv_local_set{
        { KLV_0601_MISSION_ID, std::string{ "ID_1" } } } },
      { KLV_0601_AMEND_LOCAL_SET, klv_local_set{
        { KLV_0601_MISSION_ID, std::string{ "ID_2" } } } } } } } );
  klv_md->add< kv::VITAL_META_UNIX_TIMESTAMP >( 42 );

  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 1, output.size() );
  auto const output_klv =
    dynamic_cast< klv_metadata* >( output.at( 0 ).get() );
  ASSERT_NE( nullptr, output_klv );
  ASSERT_EQ( 1, output_klv->klv().size() );
  EXPECT_EQ(
    42, output_klv->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
}

// ----------------------------------------------------------------------------
// Nested segment sets.
TEST ( apply_child_klv, segment_only )
{
  apply_child_klv filter;
  auto const klv_md = std::make_shared< klv_metadata >();
  kv::metadata_vector input{ klv_md };

  klv_md->set_klv( {
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } },
      { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
        { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM_ALT" } },
        { KLV_0601_MISSION_ID, std::string{ "ID_1" } } } },
      { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
        { KLV_0601_MISSION_ID, std::string{ "ID_2" } } } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM2" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } },
      { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
        { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM2_ALT" } },
        { KLV_0601_MISSION_ID, std::string{ "ID_1" } },
        { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
          { KLV_0601_MISSION_ID, std::string{ "ID_2" } } } },
        { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
          { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM2_ALT2" } },
          { KLV_0601_MISSION_ID, std::string{ "ID_3" } } } } } },
      { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
        { KLV_0601_MISSION_ID, {} } } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM3" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } } } );
  klv_md->add< kv::VITAL_META_UNIX_TIMESTAMP >( 42 );

  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 1, output.size() );
  auto const output_klv =
    dynamic_cast< klv_metadata* >( output.at( 0 ).get() );
  EXPECT_EQ(
    42, output_klv->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );

  std::vector< klv_packet > expected_klv = {
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM_ALT" } },
      { KLV_0601_MISSION_ID, std::string{ "ID_1" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM" } },
      { KLV_0601_MISSION_ID, std::string{ "ID_2" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM2" } },
      { KLV_0601_MISSION_ID, {} },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM2_ALT" } },
        { KLV_0601_MISSION_ID, std::string{ "ID_2" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM2_ALT2" } },
      { KLV_0601_MISSION_ID, std::string{ "ID_3" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM3" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } } };

  EXPECT_EQ(
    std::multiset< klv_packet >( expected_klv.begin(), expected_klv.end() ),
    std::multiset< klv_packet >(
      output_klv->klv().begin(), output_klv->klv().end() ) );
}

// ----------------------------------------------------------------------------
// Nested segment and amend sets.
TEST ( apply_child_klv, mixed_children )
{
  apply_child_klv filter;
  auto const klv_md = std::make_shared< klv_metadata >();
  kv::metadata_vector input{ klv_md };

  klv_md->set_klv( {
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } },
      { KLV_0601_AMEND_LOCAL_SET, klv_local_set{
        { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
          { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM_AMEND" } },
          { KLV_0601_AMEND_LOCAL_SET, klv_local_set{
            { KLV_0601_MISSION_ID, std::string{ "ID_3" } } } } } } } },
      { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
        { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM_ALT" } },
        { KLV_0601_MISSION_ID, std::string{ "ID_1" } } } },
      { KLV_0601_SEGMENT_LOCAL_SET, klv_local_set{
        { KLV_0601_MISSION_ID, std::string{ "ID_2" } } } } } } } );
  klv_md->add< kv::VITAL_META_UNIX_TIMESTAMP >( 42 );

  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 1, output.size() );
  auto const output_klv =
    dynamic_cast< klv_metadata* >( output.at( 0 ).get() );
  EXPECT_EQ(
    42, output_klv->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );

  std::vector< klv_packet > expected_klv = {
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM_ALT" } },
      { KLV_0601_MISSION_ID, std::string{ "ID_1" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM" } },
      { KLV_0601_MISSION_ID, std::string{ "ID_2" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } },
    { klv_0601_key(), klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 42 } },
      { KLV_0601_PLATFORM_DESIGNATION, std::string{ "PLATFORM_AMEND" } },
      { KLV_0601_MISSION_ID, std::string{ "ID_3" } },
      { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } } } } };

  EXPECT_EQ(
    std::multiset< klv_packet >( expected_klv.begin(), expected_klv.end() ),
    std::multiset< klv_packet >(
      output_klv->klv().begin(), output_klv->klv().end() ) );
}
