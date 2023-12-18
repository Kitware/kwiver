// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the update_klv filter.

#include "data_format.h"

#include <vital/plugin_management/plugin_manager.h>
#include <vital/algo/algorithm.txx>

#include <arrows/klv/update_klv.h>
#include <arrows/klv/klv_metadata.h>
#include <arrows/klv/klv_1108.h>
#include <arrows/klv/klv_1108_metric_set.h>

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
class update_klv_fixture : public ::testing::Test
{
protected:
  // We have to treat the timestamp fields specially, since they should be the
  // current time and we can't hardcode the correct value for that.
  void check_metric_times( klv_local_set& set ) const
  {
    for( auto& entry : set.all_at( KLV_1108_METRIC_LOCAL_SET ) )
    {
      auto& metric_set = entry.second.get< klv_local_set >();
      ASSERT_EQ( 1, metric_set.count( KLV_1108_METRIC_SET_TIME ) );

      // Check that timestamp is sane - some time between when this was written
      // and 2100.
      auto const timestamp =
        metric_set.at( KLV_1108_METRIC_SET_TIME ).get< uint64_t >();
      EXPECT_GT( timestamp, 1670000000000000);
      EXPECT_LT( timestamp, 4102462800000000);

      // Remove the timestamp field
      metric_set.erase( KLV_1108_METRIC_SET_TIME );
    }
  }

  kv::metadata_sptr
  metric_metadata(
    uint64_t timestamp, double gsd_value, double vniirs_value ) const
  {
    auto const result = new klv_metadata;
    result->add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 1 );
    result->add< kv::VITAL_META_UNIX_TIMESTAMP >( timestamp );
    result->add< kv::VITAL_META_AVERAGE_GSD >( gsd_value );
    result->add< kv::VITAL_META_VNIIRS >( vniirs_value );
    result->add< kv::VITAL_META_VIDEO_BITRATE >( 500000 );
    result->add< kv::VITAL_META_VIDEO_COMPRESSION_TYPE >( "H.264" );
    result->add< kv::VITAL_META_VIDEO_COMPRESSION_PROFILE >( "Main" );
    result->add< kv::VITAL_META_VIDEO_COMPRESSION_LEVEL >( "4.1" );
    result->add< kv::VITAL_META_VIDEO_FRAME_RATE >( 30.0 );
    result->add< kv::VITAL_META_IMAGE_WIDTH >( 1280 );
    result->add< kv::VITAL_META_IMAGE_HEIGHT >( 720 );
    return kv::metadata_sptr{ result };
  }

  klv_packet
  metric_klv(
    klv_1108_metric_period_pack const& period_pack,
    double gsd_value, double vniirs_value ) const
  {
    return { klv_1108_key(), klv_local_set{
      { KLV_1108_ASSESSMENT_POINT, KLV_1108_ASSESSMENT_POINT_ARCHIVE },
      { KLV_1108_METRIC_PERIOD_PACK, period_pack },
      { KLV_1108_METRIC_LOCAL_SET, klv_local_set{
        { KLV_1108_METRIC_SET_NAME, std::string{ "GSD" } },
        { KLV_1108_METRIC_SET_VERSION, std::string{} },
        { KLV_1108_METRIC_SET_IMPLEMENTER,
          klv_1108_kwiver_metric_implementer() },
        { KLV_1108_METRIC_SET_PARAMETERS, std::string{
          "Geo. mean of horiz. and vert. GSD of central pixel" } },
        { KLV_1108_METRIC_SET_VALUE, klv_lengthy< double >{ gsd_value } } } },
      { KLV_1108_METRIC_LOCAL_SET, klv_local_set{
        { KLV_1108_METRIC_SET_NAME, std::string{ "VNIIRS" } },
        { KLV_1108_METRIC_SET_VERSION, std::string{ "GIQE5" } },
        { KLV_1108_METRIC_SET_IMPLEMENTER,
          klv_1108_kwiver_metric_implementer() },
        { KLV_1108_METRIC_SET_PARAMETERS, std::string{ "Terms a0, a1 only" } },
        { KLV_1108_METRIC_SET_VALUE, klv_lengthy< double >{ vniirs_value } } } },
      { KLV_1108_COMPRESSION_TYPE, KLV_1108_COMPRESSION_TYPE_H264 },
      { KLV_1108_COMPRESSION_PROFILE, KLV_1108_COMPRESSION_PROFILE_MAIN },
      { KLV_1108_COMPRESSION_LEVEL, std::string{ "4.1" } },
      { KLV_1108_COMPRESSION_RATIO, klv_lengthy< double >{ 1327.104 } },
      { KLV_1108_STREAM_BITRATE, uint64_t{ 500 } },
      { KLV_1108_DOCUMENT_VERSION, uint64_t{ 3 } } } };
  }

  void expect_empty_frames( size_t count )
  {
    for( size_t i = 0; i < count; ++i )
    {
      auto const output = filter.receive();
      ASSERT_EQ( 1, output.size() );

      auto output_klv =
        dynamic_cast< klv_metadata const& >( *output[ 0 ] ).klv();
      EXPECT_EQ( 0, output_klv.size() );
    }
  }

  update_klv filter;
};

// ----------------------------------------------------------------------------
// Ensure we can create the filter with the factory method.
TEST_F ( update_klv_fixture, create )
{
  EXPECT_NE(
    nullptr, kv::create_algorithm<kv::algo::buffered_metadata_filter>( "update_klv" ) );
}

// ----------------------------------------------------------------------------
// No metadata given.
TEST_F ( update_klv_fixture, empty )
{
  kv::metadata_vector input;

  filter.send( input, nullptr );
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 1, filter.available_frames() );

  filter.flush();
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 1, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( input, output );
  EXPECT_EQ( 0, filter.available_frames() );
}

// ----------------------------------------------------------------------------
// No metadata given.
TEST_F ( update_klv_fixture, empty_with_delay )
{
  auto config = filter.get_configuration();
  config->set_value< size_t >( "st1108_frequency", 3 );
  filter.set_configuration( config );

  kv::metadata_vector input;

  filter.send( input, nullptr );
  EXPECT_EQ( 1, filter.unavailable_frames() );
  EXPECT_EQ( 0, filter.available_frames() );

  filter.flush();
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 1, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( input, output );
  EXPECT_EQ( 0, filter.available_frames() );
}

// ----------------------------------------------------------------------------
// Null metadata pointer.
TEST_F ( update_klv_fixture, null_metadata )
{
  kv::metadata_vector input{ nullptr };

  filter.send( input, nullptr );
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 1, filter.available_frames() );

  filter.flush();
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 1, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( input, output );
}

// ----------------------------------------------------------------------------
// No metadata given.
TEST_F ( update_klv_fixture, null_metadata_with_delay )
{
  auto config = filter.get_configuration();
  config->set_value< size_t >( "st1108_frequency", 3 );
  filter.set_configuration( config );

  kv::metadata_vector input{ nullptr };

  filter.send( input, nullptr );
  EXPECT_EQ( 1, filter.unavailable_frames() );
  EXPECT_EQ( 0, filter.available_frames() );

  filter.flush();
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 1, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( input, output );
  EXPECT_EQ( 0, filter.available_frames() );
}

// ----------------------------------------------------------------------------
// Metadata objects with no KLV attached.
TEST_F ( update_klv_fixture, non_klv_metadata )
{
  kv::metadata_vector input{
    std::make_shared< kv::metadata >(),
    std::make_shared< kv::metadata >(),
  };
  input[ 0 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 0 );
  input[ 0 ]->add< kv::VITAL_META_AVERAGE_GSD >( 12.0 );
  input[ 1 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 1 );

  filter.send( input, nullptr );
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 1, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( 0, filter.available_frames() );
  ASSERT_EQ( 2, output.size() );
  EXPECT_EQ(
    0, output.at( 0 )->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
  EXPECT_EQ(
    1, output.at( 1 )->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
}

// ----------------------------------------------------------------------------
// Adding in a new ST1108 packet.
TEST_F ( update_klv_fixture, add_st1108 )
{
  kv::metadata_vector input{ metric_metadata( 1, 12.0, 5.0 ) };

  filter.send( input, nullptr );
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 1, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( 0, filter.available_frames() );
  ASSERT_EQ( 1, output.size() );

  auto output_klv =
    dynamic_cast< klv_metadata const& >( *output[ 0 ] ).klv();
  ASSERT_EQ( 1, output_klv.size() );

  CALL_TEST( check_metric_times, output_klv[ 0 ].value.get< klv_local_set >() );

  std::vector< klv_packet > expected_klv {
    metric_klv( { 1, 33333 }, 12.0, 5.0 )
  };

  EXPECT_EQ( expected_klv, output_klv );
}

// ----------------------------------------------------------------------------
// Adding in a new ST1108 packet.
TEST_F ( update_klv_fixture, add_st1108_with_sample_delay )
{
  auto config = filter.get_configuration();
  config->set_value< size_t >( "st1108_frequency", 3 );
  config->set_value< std::string >( "st1108_inter", "sample" );
  filter.set_configuration( config );

  filter.send( { metric_metadata( 1, 12.0, 5.0 ) }, nullptr );
  filter.send( { metric_metadata( 33334, 13.0, 6.0 ) }, nullptr );
  filter.send( { metric_metadata( 66667, 14.0, 7.0 ) }, nullptr );
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 3, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( 2, filter.available_frames() );
  ASSERT_EQ( 1, output.size() );

  auto output_klv =
    dynamic_cast< klv_metadata const& >( *output[ 0 ] ).klv();
  ASSERT_EQ( 1, output_klv.size() );

  CALL_TEST( check_metric_times, output_klv[ 0 ].value.get< klv_local_set >() );

  std::vector< klv_packet > expected_klv{
    metric_klv( { 1, 33333 }, 12.0, 5.0 )
  };

  EXPECT_EQ( expected_klv, output_klv );

  CALL_TEST( expect_empty_frames, 2 );
}

// ----------------------------------------------------------------------------
// Adding in a new ST1108 packet.
TEST_F ( update_klv_fixture, add_st1108_with_sample_smear_delay )
{
  auto config = filter.get_configuration();
  config->set_value< size_t >( "st1108_frequency", 3 );
  config->set_value< std::string >( "st1108_inter", "sample_smear" );
  filter.set_configuration( config );

  filter.send( { metric_metadata( 1, 12.0, 5.0 ) }, nullptr );
  filter.send( { metric_metadata( 33334, 13.0, 6.0 ) }, nullptr );
  filter.send( { metric_metadata( 66667, 14.0, 7.0 ) }, nullptr );
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 3, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( 2, filter.available_frames() );
  ASSERT_EQ( 1, output.size() );

  auto output_klv =
    dynamic_cast< klv_metadata const& >( *output[ 0 ] ).klv();
  ASSERT_EQ( 1, output_klv.size() );

  CALL_TEST( check_metric_times, output_klv[ 0 ].value.get< klv_local_set >() );

  std::vector< klv_packet > expected_klv{
    metric_klv( { 1, 99999 }, 12.0, 5.0 )
  };

  EXPECT_EQ( expected_klv, output_klv );

  CALL_TEST( expect_empty_frames, 2 );
}

// ----------------------------------------------------------------------------
// Adding in a new ST1108 packet.
TEST_F ( update_klv_fixture, add_st1108_with_mean_delay )
{
  auto config = filter.get_configuration();
  config->set_value< size_t >( "st1108_frequency", 3 );
  config->set_value< std::string >( "st1108_inter", "mean" );
  filter.set_configuration( config );

  filter.send( { metric_metadata( 1, 12.0, 5.0 ) }, nullptr );
  filter.send( { metric_metadata( 33334, 13.0, 6.0 ) }, nullptr );
  filter.send( { metric_metadata( 66667, 17.0, 10.0 ) }, nullptr );
  EXPECT_EQ( 0, filter.unavailable_frames() );
  EXPECT_EQ( 3, filter.available_frames() );

  auto const output = filter.receive();
  EXPECT_EQ( 2, filter.available_frames() );
  ASSERT_EQ( 1, output.size() );

  auto output_klv =
    dynamic_cast< klv_metadata const& >( *output[ 0 ] ).klv();
  ASSERT_EQ( 1, output_klv.size() );

  CALL_TEST( check_metric_times, output_klv[ 0 ].value.get< klv_local_set >() );

  std::vector< klv_packet > expected_klv{
    metric_klv( { 1, 99999 }, 14.0, 7.0 )
  };

  EXPECT_EQ( expected_klv, output_klv );

  CALL_TEST( expect_empty_frames, 2 );
}
