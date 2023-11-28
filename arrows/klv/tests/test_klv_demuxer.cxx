// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV demuxer.

#include <arrows/klv/klv_all.h>
#include <arrows/klv/klv_demuxer.h>

#include <tests/test_gtest.h>

#include <optional>

namespace kv = kwiver::vital;

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
  using packet_set = std::set< klv_packet >;
  klv_uds_key const key1{ 0x060E2B34FFFFFFFF, 0x0A0B0C0D00000000 };
  klv_uds_key const key2{ 0x060E2B34FFFFFFFF, 0x0000000000000000 };
  klv_blob const data1{ { 0xAA, 0xBB, 0xCC, 0xDD } };
  klv_blob const data2{ { 0xAA, 0xBB } };
  klv_blob const data3{ { 0xAB, 0xCD } };
  auto const packets = std::vector< klv_packet >{
    { key1, data1 },
    { key1, data2 },
    { key2, data3 },
    { klv_0601_key(), klv_blob{ 0x00 } }, };

  klv_timeline timeline;
  klv_demuxer demuxer( timeline );
  for( auto const& packet : packets )
  {
    demuxer.send_frame( { packet }, 123 );
  }

  {
    klv_timeline reverse_timeline;
    klv_demuxer reverse_demuxer( reverse_timeline );
    for( auto it = packets.rbegin(); it != packets.rend(); ++it )
    {
      reverse_demuxer.send_frame( { *it }, 123 );
    }

    EXPECT_EQ( timeline, reverse_timeline );
  }

  auto const result_range = timeline.find_all( KLV_PACKET_UNKNOWN, 0 );
  ASSERT_EQ( 3, std::distance( result_range.begin(), result_range.end() ) );
  EXPECT_EQ( packet_set( { packets[ 3 ] } ),
             std::next( result_range.begin(), 0 )->second.at( 123 )
                                                 ->get< packet_set >() );
  EXPECT_EQ( packet_set( { packets[ 2 ] } ),
             std::next( result_range.begin(), 1 )->second.at( 123 )
                                                 ->get< packet_set >() );
  EXPECT_EQ( packet_set( { packets[ 0 ], packets[ 1 ] } ),
             std::next( result_range.begin(), 2 )->second.at( 123 )
                                                 ->get< packet_set >() );
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
          klv_blob{ 0xAA } },
        { KLV_0601_PLATFORM_DESIGNATION,           // Repeated but unchanged
          std::string{ "Bob" } } } },
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
    demuxer.send_frame( { packet } );
  }

  {
    klv_timeline reverse_timeline;
    klv_demuxer reverse_demuxer( reverse_timeline );
    for( auto it = packets.rbegin(); it != packets.rend(); ++it )
    {
      reverse_demuxer.send_frame( { *it } );
    }

    EXPECT_EQ( timeline, reverse_timeline );
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
  EXPECT_EQ( klv_blob{ 0xAA },
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
TEST ( klv, demuxer_0601_special )
{
  auto const packets = std::vector< klv_packet >{
    { klv_0601_key(),
      klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 15 } },
        { KLV_0601_WAVELENGTHS_LIST,
          std::vector< klv_0601_wavelength_record >{
            { 1, 380.0, 750.0, "VIS" },
            { 2, 750.0, 100000.0, "IR" } } },
        { KLV_0601_PAYLOAD_LIST,
          std::vector< klv_0601_payload_record >{
            { 0, KLV_0601_PAYLOAD_TYPE_ELECTRO_OPTICAL, "VIS Nose Camera" },
            { 1, KLV_0601_PAYLOAD_TYPE_ELECTRO_OPTICAL, "ACME VIS" } } },
        { KLV_0601_WAYPOINT_LIST,
          std::vector< klv_0601_waypoint_record >{
            { 0, 1, std::nullopt, std::nullopt },
            { 1, 2, std::nullopt, std::nullopt } } },
        { KLV_0601_WEAPON_FIRED, uint64_t{ 0xBA } },
        { KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST,
          std::vector< uint64_t >{ 0 } },
        { KLV_0601_SEGMENT_LOCAL_SET,
          klv_local_set{
            { KLV_0601_MISSION_ID, std::string{ "MISSION01" } } } },
        { KLV_0601_SEGMENT_LOCAL_SET,
          klv_local_set{
            { KLV_0601_MISSION_ID, std::string{ "MISSION02" } } } },
        { KLV_0601_AMEND_LOCAL_SET,
          klv_local_set{
            { KLV_0601_WEAPON_FIRED, uint64_t{ 0xBB } } } },
        { KLV_0601_AMEND_LOCAL_SET,
          klv_local_set{
            { KLV_0601_LASER_PRF_CODE, uint64_t{ 1111 } } } },
        { KLV_0601_SDCC_FLP,
          klv_1010_sdcc_flp{
            { KLV_0601_SENSOR_LATITUDE, KLV_0601_SENSOR_LONGITUDE },
            { 1.0, 2.0 },
            {} } },
        { KLV_0601_SDCC_FLP,
          klv_1010_sdcc_flp{
            { KLV_0601_ALTERNATE_PLATFORM_LATITUDE,
              KLV_0601_ALTERNATE_PLATFORM_LONGITUDE },
            { 2.0, 3.0 },
            {} } },
        { KLV_0601_CONTROL_COMMAND,
          klv_0601_control_command{ 0, "CMD0", 12 } },
        { KLV_0601_CONTROL_COMMAND,
          klv_0601_control_command{ 1, "CMD1", 13 } } } },
    { klv_0601_key(),
      klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 30 } },
        { KLV_0601_WAVELENGTHS_LIST,
          std::vector< klv_0601_wavelength_record >{
            { 1, 380.0, 750.0, "VIS" },
            { 3, 380.0, 750.0, "VIS2" },
            { 4, 750.0, 100000.0, "IR2" } } },
        { KLV_0601_PAYLOAD_LIST,
          std::vector< klv_0601_payload_record >{
            { 0, KLV_0601_PAYLOAD_TYPE_ELECTRO_OPTICAL, "VIS Nose Camera" },
            { 2, KLV_0601_PAYLOAD_TYPE_ELECTRO_OPTICAL, "VIS Nose Camera 2" },
            { 3, KLV_0601_PAYLOAD_TYPE_ELECTRO_OPTICAL, "ACME VIS 2" } } },
        { KLV_0601_WAYPOINT_LIST,
          std::vector< klv_0601_waypoint_record >{
            { 0, 1, std::nullopt, std::nullopt },
            { 2, 3, std::nullopt, std::nullopt },
            { 3, 4, std::nullopt, std::nullopt } } },
        { KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST,
          std::vector< uint64_t >{ 1 } },
        { KLV_0601_SDCC_FLP,
          klv_1010_sdcc_flp{
            { KLV_0601_ALTERNATE_PLATFORM_LATITUDE,
              KLV_0601_ALTERNATE_PLATFORM_LONGITUDE },
            { 12.0, 13.0 },
            {} } },
        { KLV_0601_CONTROL_COMMAND,
          klv_0601_control_command{ 1, "CMD1", 13 } } } } };

  klv_timeline timeline;
  klv_demuxer demuxer( timeline );
  for( auto const& packet : packets )
  {
    demuxer.send_frame( { packet } );
  }

  auto const standard = KLV_PACKET_MISB_0601_LOCAL_SET;

  // Lists
  {
    auto const tag = KLV_0601_WAVELENGTHS_LIST;
    auto const slice = timeline.all_at( standard, tag, 15 );
    ASSERT_EQ( slice.size(), 2 );
    EXPECT_EQ( 1, slice.at( 0 ).get< klv_0601_wavelength_record >().id );
    EXPECT_EQ( 2, slice.at( 1 ).get< klv_0601_wavelength_record >().id );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 16 ).size() );
    EXPECT_EQ( 4, timeline.all_at( standard, tag, 30 ).size() );
  }

  {
    auto const tag = KLV_0601_PAYLOAD_LIST;
    auto const slice = timeline.all_at( standard, tag, 15 );
    ASSERT_EQ( slice.size(), 2 );
    EXPECT_EQ( 0, slice.at( 0 ).get< klv_0601_payload_record >().id );
    EXPECT_EQ( 1, slice.at( 1 ).get< klv_0601_payload_record >().id );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 16 ).size() );
    EXPECT_EQ( 4, timeline.all_at( standard, tag, 30 ).size() );
  }

  {
    auto const tag = KLV_0601_WAYPOINT_LIST;
    auto const slice = timeline.all_at( standard, tag, 15 );
    ASSERT_EQ( 2, slice.size() );
    EXPECT_EQ( 0, slice.at( 0 ).get< klv_0601_waypoint_record >().id );
    EXPECT_EQ( 1, slice.at( 1 ).get< klv_0601_waypoint_record >().id );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 16 ).size() );
    EXPECT_EQ( 4, timeline.all_at( standard, tag, 30 ).size() );
  }

  // Points with single entries
  {
    auto const tag = KLV_0601_WEAPON_FIRED;
    EXPECT_EQ( uint64_t{ 0xBA }, timeline.at( standard, tag, 15 ) );
    EXPECT_TRUE( timeline.at( standard, tag, 16 ).empty() );
  }

  {
    auto const tag = KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST;
    EXPECT_EQ( std::vector< uint64_t >{ 0 },
               timeline.at( standard, tag, 15 ) );
    EXPECT_TRUE( timeline.at( standard, tag, 16 ).empty() );
    EXPECT_EQ( std::vector< uint64_t >{ 1 },
               timeline.at( standard, tag, 30 ) );
    EXPECT_TRUE( timeline.at( standard, tag, 31 ).empty() );
  }

  // Points with multiple entries
  {
    auto const tag = KLV_0601_SEGMENT_LOCAL_SET;
    auto const slice = timeline.all_at( standard, tag, 15 );
    ASSERT_EQ( 2, slice.size() );
    EXPECT_EQ(
      std::string{ "MISSION01" },
      slice.at( 0 ).get< klv_local_set >().at( KLV_0601_MISSION_ID ) );
    EXPECT_EQ(
      std::string{ "MISSION02" },
      slice.at( 1 ).get< klv_local_set >().at( KLV_0601_MISSION_ID ) );
    EXPECT_TRUE( timeline.at( standard, tag, 16 ).empty() );
  }

  {
    auto const tag = KLV_0601_AMEND_LOCAL_SET;
    auto const slice = timeline.all_at( standard, tag, 15 );
    ASSERT_EQ( 2, slice.size() );
    EXPECT_EQ(
      uint64_t{ 0xBB },
      slice.at( 0 ).get< klv_local_set >().at( KLV_0601_WEAPON_FIRED ) );
    EXPECT_EQ(
      uint64_t{ 1111 },
      slice.at( 1 ).get< klv_local_set >().at( KLV_0601_LASER_PRF_CODE ) );
    EXPECT_TRUE( timeline.at( standard, tag, 16 ).empty() );
  }

  // Standard multi-entries
  {
    auto const tag = KLV_0601_SDCC_FLP;
    auto const slice1 = timeline.all_at( standard, tag, 15 );
    ASSERT_EQ( 2, slice1.size() );
    EXPECT_EQ( std::vector< double >( { 1.0, 2.0 } ),
               slice1.at( 0 ).get< klv_1010_sdcc_flp >().sigma );
    EXPECT_EQ( std::vector< double >( { 2.0, 3.0 } ),
               slice1.at( 1 ).get< klv_1010_sdcc_flp >().sigma );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 16 ).size() );
    ASSERT_EQ( 2, timeline.all_at( standard, tag, 30 ).size() );

    auto const slice2 = timeline.all_at( standard, tag, 30 );
    ASSERT_EQ( 2, slice2.size() );
    EXPECT_EQ( std::vector< double >( { 1.0, 2.0 } ),
               slice2.at( 0 ).get< klv_1010_sdcc_flp >().sigma );
    EXPECT_EQ( std::vector< double >( { 12.0, 13.0 } ),
               slice2.at( 1 ).get< klv_1010_sdcc_flp >().sigma );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 31 ).size() );
  }

  {
    auto const tag = KLV_0601_CONTROL_COMMAND;
    auto const slice = timeline.all_at( standard, tag, 15 );
    ASSERT_EQ( 2, slice.size() );
    EXPECT_EQ( uint64_t{ 0 },
               slice.at( 0 ).get< klv_0601_control_command >().id );
    EXPECT_EQ( uint64_t{ 1 },
               slice.at( 1 ).get< klv_0601_control_command >().id );
    EXPECT_EQ( 2, timeline.all_at( standard, tag, 16 ).size() );
    ASSERT_EQ( 2, timeline.all_at( standard, tag, 30 ).size() );
  }
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
        { KLV_1108_METRIC_LOCAL_SET,    klv_blob{ 0xAA } } } } };

  klv_timeline timeline;
  klv_demuxer demuxer( timeline );
  for( auto const& packet : packets )
  {
    demuxer.send_frame( { packet } );
  }

  {
    klv_timeline reverse_timeline;
    klv_demuxer reverse_demuxer( reverse_timeline );
    for( auto it = packets.rbegin(); it != packets.rend(); ++it )
    {
      reverse_demuxer.send_frame( { *it } );
    }

    EXPECT_EQ( timeline, reverse_timeline );
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
  EXPECT_EQ( std::vector< klv_value >( { std::string{ "5.2" },
                                         std::string{ "5.1" },
                                         std::string{ "5.1" } } ),
             timeline.all_at( standard, KLV_1108_COMPRESSION_LEVEL, 155 ) );
}

// ----------------------------------------------------------------------------
TEST ( klv, demuxer_multipacket_frame )
{
  auto const packets1 = std::vector< klv_packet >{
    { { 1, 2 }, klv_blob{ { 0xFF } } },
    { klv_0601_key(),
      klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 100 } },
        { KLV_0601_MISSION_ID, std::string{ "TEST1" } } } },
    { klv_0601_key(),
      klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 200 } },
        { KLV_0601_MISSION_ID, std::string{ "TEST2" } } } },
    { { 3, 4 }, klv_blob{ { 0xFF } } },
  };

  auto const packets2 = std::vector< klv_packet >{
    { { 5, 6 }, klv_blob{ { 0xFF } } },
    { klv_0601_key(),
      klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 400 } },
        { KLV_0601_MISSION_ID, std::string{ "TEST4" } } } },
    { klv_0601_key(),
      klv_local_set{
        { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 300 } },
        { KLV_0601_MISSION_ID, std::string{ "TEST3" } } } },
    { { 7, 8 }, klv_blob{ { 0xFF } } },
  };

  klv_timeline timeline;
  klv_demuxer demuxer( timeline );
  demuxer.send_frame( packets1 );
  demuxer.send_frame( packets2 );

  {
    klv_timeline reverse_timeline;
    klv_demuxer reverse_demuxer( reverse_timeline );
    reverse_demuxer.send_frame( packets2 );
    reverse_demuxer.send_frame( packets1 );

    EXPECT_EQ( timeline, reverse_timeline );
  }
}
