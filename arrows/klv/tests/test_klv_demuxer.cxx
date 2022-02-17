// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test KLV demuxer.

#include <arrows/klv/klv_0601.h>
#include <arrows/klv/klv_1108.h>
#include <arrows/klv/klv_1108_metric_set.h>
#include <arrows/klv/klv_demuxer.h>

#include <tests/test_gtest.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

using namespace kwiver::arrows::klv;

// ----------------------------------------------------------------------------
TEST ( klv, demuxer_invalid )
{
  // Unknown UDS keys or unparsed data
  {
    using packet_vector = std::vector< klv_packet >;
    klv_uds_key const key1{ 0x060E2B34FFFFFFFF, 0x0A0B0C0D00000000 };
    klv_uds_key const key2{ 0x060E2B34FFFFFFFF, 0x0000000000000000 };
    klv_blob const data1{ { 0xAA, 0xBB, 0xCC, 0xDD } };
    klv_blob const data2{ { 0xAA, 0xBB } };
    klv_blob const data3{ { 0xAB, 0xCD } };
    auto const packets = packet_vector{
      { key1, data1 },
      { key1, data2 },
      { key2, data3 },
      { klv_0601_key(), klv_blob{ { 0x00 } } }, };

    klv_timeline timeline;
    klv_demuxer demuxer( timeline );
    demuxer.seek( 123 );
    for( auto const& packet : packets )
    {
      demuxer.demux_packet( packet );
    }
    auto const result_range = timeline.find_all( KLV_PACKET_UNKNOWN, 0 );
    ASSERT_EQ( 3, std::distance( result_range.begin(), result_range.end() ) );
    EXPECT_EQ( packet_vector( { packets[ 0 ], packets[ 1 ] } ),
               std::next( result_range.begin(), 0 )->second.at( 123 )->get< packet_vector >() );
    EXPECT_EQ( packet_vector( { packets[ 2 ] } ),
               std::next( result_range.begin(), 1 )->second.at( 123 )->get< packet_vector >() );
    EXPECT_EQ( packet_vector( { packets[ 3 ] } ),
               std::next( result_range.begin(), 2 )->second.at( 123 )->get< packet_vector >() ); }
}

// ----------------------------------------------------------------------------
TEST ( klv, demuxer_0601 )
{
  auto const packets = std::vector< klv_packet >{
    { klv_0601_key(),
      klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 10 } },
        { KLV_0601_ICING_DETECTED, KLV_0601_ICING_DETECTED_FALSE },
        { KLV_0601_PLATFORM_HEADING_ANGLE, 13.0 },
        { KLV_0601_LASER_PRF_CODE, uint64_t{ 1111 } },
        { KLV_0601_PLATFORM_CALL_SIGN, std::string{ "BOB" } },
        { KLV_0601_PLATFORM_DESIGNATION, std::string{ "Bob" } } } },
    { klv_0601_key(),
      klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 20 } },
        // Implicitly unchanged
        { KLV_0601_PLATFORM_HEADING_ANGLE, 14.0 }, // Explicitly changed
        { KLV_0601_LASER_PRF_CODE, klv_value{} },  // Explicitly erased
        { KLV_0601_PLATFORM_CALL_SIGN,             // Changed to invalid
          klv_blob{ { 0xAA } } },
        { KLV_0601_PLATFORM_DESIGNATION,           // Repeated but unchanged
          std::string{ "Bob" } }
      } },
    { klv_0601_key(),
      klv_local_set{
        // All new values
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 30 } },
        { KLV_0601_ICING_DETECTED, KLV_0601_ICING_DETECTED_TRUE },
        { KLV_0601_PLATFORM_HEADING_ANGLE, 15.0 },
        { KLV_0601_LASER_PRF_CODE, uint64_t{ 2222 } },
        { KLV_0601_PLATFORM_CALL_SIGN, std::string{ "ALICE" } },
        { KLV_0601_PLATFORM_DESIGNATION, std::string{ "Alice" } } } }, };

  klv_timeline timeline;
  klv_demuxer demuxer( timeline );
  for( auto const& packet : packets )
  {
    demuxer.demux_packet( packet );
  }

  auto const standard = KLV_PACKET_MISB_0601_LOCAL_SET;

  // Before assignment
  EXPECT_TRUE( timeline.at( standard, KLV_0601_ICING_DETECTED,         9 )
               .empty() );
  EXPECT_TRUE( timeline.at( standard, KLV_0601_PLATFORM_HEADING_ANGLE, 9 )
               .empty() );
  EXPECT_TRUE( timeline.at( standard, KLV_0601_LASER_PRF_CODE,         9 )
               .empty() );
  EXPECT_TRUE( timeline.at( standard, KLV_0601_PLATFORM_CALL_SIGN,     9 )
               .empty() );
  EXPECT_TRUE( timeline.at( standard, KLV_0601_PLATFORM_DESIGNATION,   9 )
               .empty() );

  // After first assignment
  EXPECT_EQ( KLV_0601_ICING_DETECTED_FALSE,
             timeline.at( standard, KLV_0601_ICING_DETECTED,         10 ) );
  EXPECT_EQ( 13.0,
             timeline.at( standard, KLV_0601_PLATFORM_HEADING_ANGLE, 10 ) );
  EXPECT_EQ( uint64_t{ 1111 },
             timeline.at( standard, KLV_0601_LASER_PRF_CODE,         10 ) );
  EXPECT_EQ( std::string{ "BOB" },
             timeline.at( standard, KLV_0601_PLATFORM_CALL_SIGN,     10 ) );
  EXPECT_EQ( std::string{ "Bob" },
             timeline.at( standard, KLV_0601_PLATFORM_DESIGNATION,   10 ) );

  // After tricky assignments
  EXPECT_EQ( KLV_0601_ICING_DETECTED_FALSE,
             timeline.at( standard, KLV_0601_ICING_DETECTED,         20 ) );
  EXPECT_EQ( 14.0,
             timeline.at( standard, KLV_0601_PLATFORM_HEADING_ANGLE, 20 ) );
  EXPECT_EQ( klv_value{},
             timeline.at( standard, KLV_0601_LASER_PRF_CODE,         20 ) );
  EXPECT_EQ( klv_blob{ { 0xAA } },
             timeline.at( standard, KLV_0601_PLATFORM_CALL_SIGN,     20 ) );
  EXPECT_EQ( std::string{ "Bob" },
             timeline.at( standard, KLV_0601_PLATFORM_DESIGNATION,   20 ) );

  // After full reassignment
  EXPECT_EQ( KLV_0601_ICING_DETECTED_TRUE,
             timeline.at( standard, KLV_0601_ICING_DETECTED,         30 ) );
  EXPECT_EQ( 15.0,
             timeline.at( standard, KLV_0601_PLATFORM_HEADING_ANGLE, 30 ) );
  EXPECT_EQ( uint64_t{ 2222 },
             timeline.at( standard, KLV_0601_LASER_PRF_CODE,         30 ) );
  EXPECT_EQ( std::string{ "ALICE" },
             timeline.at( standard, KLV_0601_PLATFORM_CALL_SIGN,     30 ) );
  EXPECT_EQ( std::string{ "Alice" },
             timeline.at( standard, KLV_0601_PLATFORM_DESIGNATION,   30 ) );

  // Check final time boundary
  EXPECT_EQ( KLV_0601_ICING_DETECTED_TRUE,
             timeline.at( standard, KLV_0601_ICING_DETECTED,
                          30000029 ) );
  EXPECT_EQ( 15.0,
             timeline.at( standard, KLV_0601_PLATFORM_HEADING_ANGLE,
                          30000029 ) );
  EXPECT_EQ( uint64_t{ 2222 },
             timeline.at( standard, KLV_0601_LASER_PRF_CODE,
                          30000029 ) );
  EXPECT_EQ( std::string{ "ALICE" },
             timeline.at( standard, KLV_0601_PLATFORM_CALL_SIGN,
                          30000029 ) );
  EXPECT_EQ( std::string{ "Alice" },
             timeline.at( standard, KLV_0601_PLATFORM_DESIGNATION,
                          30000029 ) );

  EXPECT_TRUE( timeline.at( standard, KLV_0601_ICING_DETECTED,
                            30000030 ).empty() );
  EXPECT_TRUE( timeline.at( standard, KLV_0601_PLATFORM_HEADING_ANGLE,
                            30000030 ).empty() );
  EXPECT_TRUE( timeline.at( standard, KLV_0601_LASER_PRF_CODE,
                            30000030 ).empty() );
  EXPECT_TRUE( timeline.at( standard, KLV_0601_PLATFORM_CALL_SIGN,
                            30000030 ).empty() );
  EXPECT_TRUE( timeline.at( standard, KLV_0601_PLATFORM_DESIGNATION,
                            30000030 ).empty() );
}

// ----------------------------------------------------------------------------
TEST ( klv, demuxer_1108 )
{
  auto const metric_sets = std::vector< klv_local_set >{
    { { KLV_1108_METRIC_SET_NAME,       std::string{ "VNIIRS" } },
      { KLV_1108_METRIC_SET_VERSION,    std::string{ "3.0" } },
      { KLV_1108_METRIC_SET_IMPLEMENTER,
        klv_1108_metric_implementer{ "KW", "CV" } },
      { KLV_1108_METRIC_SET_PARAMETERS, std::string{ "A0+A1" } },
      { KLV_1108_METRIC_SET_TIME,       uint64_t{ 1630000000000000 } },
      { KLV_1108_METRIC_SET_VALUE,      7.0 } },
    { { KLV_1108_METRIC_SET_NAME,       std::string{ "GSD" } },
      { KLV_1108_METRIC_SET_VERSION,    klv_value{} },
      { KLV_1108_METRIC_SET_IMPLEMENTER,
        klv_1108_metric_implementer{ "KW", "CV" } },
      { KLV_1108_METRIC_SET_PARAMETERS, std::string{ "" } },
      { KLV_1108_METRIC_SET_TIME,       uint64_t{ 1630000000000000 } },
      { KLV_1108_METRIC_SET_VALUE,      9.0 } },
    { { KLV_1108_METRIC_SET_NAME,       std::string{ "VNIIRS" } },
      { KLV_1108_METRIC_SET_VERSION,    std::string{ "3.1" } },
      { KLV_1108_METRIC_SET_IMPLEMENTER,
        klv_1108_metric_implementer{ "OTHER", "OTHER" } },
      { KLV_1108_METRIC_SET_PARAMETERS, std::string{ "" } },
      { KLV_1108_METRIC_SET_TIME,       uint64_t{ 1600000000000000 } },
      { KLV_1108_METRIC_SET_VALUE,      6.0 } },
    { { KLV_1108_METRIC_SET_NAME,       std::string{ "VNIIRS" } },
      { KLV_1108_METRIC_SET_VERSION,    std::string{ "3.0" } },
      { KLV_1108_METRIC_SET_IMPLEMENTER,
        klv_1108_metric_implementer{ "KW", "CV" } },
      { KLV_1108_METRIC_SET_PARAMETERS, std::string{ "A0+A1" } },
      { KLV_1108_METRIC_SET_TIME,       uint64_t{ 1630000000000000 } },
      { KLV_1108_METRIC_SET_VALUE,      8.0 } }, };

  auto const packets = std::vector< klv_packet >{
    { klv_1108_key(),
      klv_local_set{
        { KLV_1108_ASSESSMENT_POINT,    KLV_1108_ASSESSMENT_POINT_ARCHIVE },
        { KLV_1108_METRIC_PERIOD_PACK,
          klv_1108_metric_period_pack{ 100, 100 } },
        { KLV_1108_METRIC_LOCAL_SET,    metric_sets[ 0 ] },
        { KLV_1108_METRIC_LOCAL_SET,    metric_sets[ 2 ] },
        { KLV_1108_COMPRESSION_TYPE,    KLV_1108_COMPRESSION_TYPE_H264 },
        { KLV_1108_COMPRESSION_PROFILE, KLV_1108_COMPRESSION_PROFILE_HIGH },
        { KLV_1108_COMPRESSION_LEVEL,   std::string{ "5.1" } },
        { KLV_1108_COMPRESSION_RATIO,   klv_lengthy< double >{ 25.2, 4 } },
        { KLV_1108_STREAM_BITRATE,      uint64_t{ 1024 } },
        { KLV_1108_DOCUMENT_VERSION,    uint64_t{ 3 } } } },
    { klv_1108_key(),
      klv_local_set{
        { KLV_1108_ASSESSMENT_POINT,    KLV_1108_ASSESSMENT_POINT_SENSOR },
        { KLV_1108_METRIC_PERIOD_PACK,
          klv_1108_metric_period_pack{ 150, 100 } },
        { KLV_1108_METRIC_LOCAL_SET,    metric_sets[ 1 ] },
        { KLV_1108_COMPRESSION_TYPE,    KLV_1108_COMPRESSION_TYPE_H264 },
        { KLV_1108_COMPRESSION_PROFILE, KLV_1108_COMPRESSION_PROFILE_HIGH },
        { KLV_1108_COMPRESSION_LEVEL,   std::string{ "5.2" } },
        { KLV_1108_COMPRESSION_RATIO,   klv_lengthy< double >{ 13.0, 4 } },
        { KLV_1108_STREAM_BITRATE,      uint64_t{ 1024 } },
        { KLV_1108_DOCUMENT_VERSION,    uint64_t{ 3 } } } },
    { klv_1108_key(),
      klv_local_set{
        { KLV_1108_ASSESSMENT_POINT,    KLV_1108_ASSESSMENT_POINT_ARCHIVE },
        { KLV_1108_METRIC_PERIOD_PACK,
          klv_1108_metric_period_pack{ 180, 100 } },
        { KLV_1108_METRIC_LOCAL_SET,    metric_sets[ 3 ] },
        { KLV_1108_METRIC_LOCAL_SET,    klv_blob{ { 0xAA } } } } } };

  klv_timeline timeline;
  klv_demuxer demuxer( timeline );
  for( auto const& packet : packets )
  {
    demuxer.demux_packet( packet );
  }

  auto const standard = KLV_PACKET_MISB_1108_LOCAL_SET;
  {
    auto const tag = KLV_1108_METRIC_LOCAL_SET;
    EXPECT_EQ( 0, timeline.all_at( standard, tag, 99  ).size() );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 100 ).size() );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 120 ).size() );
    EXPECT_EQ( 3, timeline.all_at( standard, tag, 150 ).size() );
    EXPECT_EQ( 4, timeline.all_at( standard, tag, 180 ).size() );
    EXPECT_EQ( 3, timeline.all_at( standard, tag, 200 ).size() );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 250 ).size() );
    EXPECT_EQ( 0, timeline.all_at( standard, tag, 280 ).size() );
  }

  EXPECT_TRUE( timeline.all_at( standard, KLV_1108_METRIC_PERIOD_PACK, 180 )
               .empty() );
  EXPECT_EQ( std::vector< klv_value >( { std::string{ "5.1" },
                                         std::string{ "5.1" },
                                         std::string{ "5.2" } } ),
             timeline.all_at( standard, KLV_1108_COMPRESSION_LEVEL, 155 ) );
}
