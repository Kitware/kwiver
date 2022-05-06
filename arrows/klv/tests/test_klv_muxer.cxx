// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV muxer.

#include <arrows/klv/klv_0601.h>
#include <arrows/klv/klv_1108.h>
#include <arrows/klv/klv_1108_metric_set.h>
#include <arrows/klv/klv_demuxer.h>
#include <arrows/klv/klv_muxer.h>

#include <tests/test_gtest.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

using namespace kwiver::arrows::klv;
namespace kv = kwiver::vital;

// ----------------------------------------------------------------------------
class klv_muxer_test : public ::testing::Test {
protected:
  klv_muxer_test() : src_timeline{}, dst_timeline{}, standard{ KLV_PACKET_UNKNOWN }, index{}, timestamps{
    kv::timestamp{ 100, 1 },
    kv::timestamp{ 110, 2 },
    kv::timestamp{ 120, 3 },
    kv::timestamp{ 130, 4 },
    kv::timestamp{ 140, 5 }, } {}

  void SetUp() {
    standard = KLV_PACKET_MISB_0601_LOCAL_SET;
    index = klv_value{};
    add_src( KLV_0601_PLATFORM_HEADING_ANGLE, { 90, 115 }, 30.0 );
    add_src( KLV_0601_PLATFORM_HEADING_ANGLE, { 125, 145 }, 40.0 );
    add_src( KLV_0601_PLATFORM_PITCH_ANGLE, { 101, 140 }, -11.0 );
    add_src( KLV_0601_PLATFORM_ROLL_ANGLE, { 121, 130 }, 9.0 );
    add_src( KLV_0601_PLATFORM_ROLL_ANGLE, { 131, 140 }, 8.0 );

    add_dst( KLV_0601_PLATFORM_HEADING_ANGLE, { 100, 120 }, 30.0 );
    add_dst( KLV_0601_PLATFORM_HEADING_ANGLE, { 130, 30000140 }, 40.0 );
    add_dst( KLV_0601_PLATFORM_PITCH_ANGLE, { 110, 140 }, -11.0 );

    standard = KLV_PACKET_MISB_1108_LOCAL_SET;
    index = klv_local_set{
      { KLV_1108_ASSESSMENT_POINT, KLV_1108_ASSESSMENT_POINT_ARCHIVE },
      { KLV_1108_METRIC_LOCAL_SET,
        klv_local_set{
          { KLV_1108_METRIC_SET_NAME, std::string{ "GSD" } },
          { KLV_1108_METRIC_SET_VERSION, std::string{""} },
          { KLV_1108_METRIC_SET_IMPLEMENTER, std::string{"KWIVER"} } } } };
    add_src( KLV_1108_ASSESSMENT_POINT, { 110, 135 }, KLV_1108_ASSESSMENT_POINT_ARCHIVE );
    add_src( KLV_1108_METRIC_LOCAL_SET, { 110, 135 }, klv_local_set{
    { KLV_1108_METRIC_SET_NAME, std::string{ "GSD" } },
    { KLV_1108_METRIC_SET_VERSION, std::string{ "" } },
    { KLV_1108_METRIC_SET_IMPLEMENTER, std::string{ "KWIVER" } },
    { KLV_1108_METRIC_SET_TIME, uint64_t{ 123456 } },
    { KLV_1108_METRIC_SET_VALUE, 20.0 }, } );
    add_src( KLV_1108_COMPRESSION_TYPE, { 110, 135 }, KLV_1108_COMPRESSION_TYPE_H264 );
    add_src( KLV_1108_COMPRESSION_PROFILE, { 110, 135 }, KLV_1108_COMPRESSION_PROFILE_MAIN );
    add_src( KLV_1108_COMPRESSION_LEVEL, { 110, 135 }, std::string{ "5.1" } );
    add_src( KLV_1108_COMPRESSION_RATIO, { 110, 135 }, 22.0  );
    add_src( KLV_1108_STREAM_BITRATE, { 110, 135 }, uint64_t{ 26 } );
    add_src( KLV_1108_DOCUMENT_VERSION, { 110, 135 }, uint64_t{ 3 } );

    add_dst( KLV_1108_ASSESSMENT_POINT, { 110, 135 }, KLV_1108_ASSESSMENT_POINT_ARCHIVE );
    add_dst( KLV_1108_METRIC_LOCAL_SET, { 110, 135 }, klv_local_set{
    { KLV_1108_METRIC_SET_NAME, std::string{ "GSD" } },
    { KLV_1108_METRIC_SET_VERSION, std::string{ "" } },
    { KLV_1108_METRIC_SET_IMPLEMENTER, std::string{ "KWIVER" } },
    { KLV_1108_METRIC_SET_TIME, uint64_t{ 123456 } },
    { KLV_1108_METRIC_SET_VALUE, 20.0 }, } );
    add_dst( KLV_1108_COMPRESSION_TYPE, { 110, 135 }, KLV_1108_COMPRESSION_TYPE_H264 );
    add_dst( KLV_1108_COMPRESSION_PROFILE, { 110, 135 }, KLV_1108_COMPRESSION_PROFILE_MAIN );
    add_dst( KLV_1108_COMPRESSION_LEVEL, { 110, 135 }, std::string{ "5.1" } );
    add_dst( KLV_1108_COMPRESSION_RATIO, { 110, 135 }, 22.0  );
    add_dst( KLV_1108_STREAM_BITRATE, { 110, 135 }, uint64_t{ 26 } );
    add_dst( KLV_1108_DOCUMENT_VERSION, { 110, 135 }, uint64_t{ 3 } );

    standard = KLV_PACKET_MISB_1108_LOCAL_SET;
    index = klv_local_set{
      { KLV_1108_ASSESSMENT_POINT, KLV_1108_ASSESSMENT_POINT_ARCHIVE },
      { KLV_1108_METRIC_LOCAL_SET,
        klv_local_set{
          { KLV_1108_METRIC_SET_NAME, std::string{"VNIIRS"} },
          { KLV_1108_METRIC_SET_VERSION, std::string{"1.0"} },
          { KLV_1108_METRIC_SET_IMPLEMENTER, std::string{"KWIVER"} } } } };
    add_src( KLV_1108_ASSESSMENT_POINT, { 110, 135 }, KLV_1108_ASSESSMENT_POINT_ARCHIVE );
    add_src( KLV_1108_METRIC_LOCAL_SET, { 110, 135 },  klv_local_set{
    { KLV_1108_METRIC_SET_NAME, std::string{ "VNIIRS" } },
    { KLV_1108_METRIC_SET_VERSION, std::string{ "1.0" } },
    { KLV_1108_METRIC_SET_IMPLEMENTER, std::string{ "KWIVER" } },
    { KLV_1108_METRIC_SET_TIME, uint64_t{ 123456 } },
    { KLV_1108_METRIC_SET_VALUE, 5.0 }, } );
    add_src( KLV_1108_COMPRESSION_TYPE, { 110, 135 }, KLV_1108_COMPRESSION_TYPE_H264 );
    add_src( KLV_1108_COMPRESSION_PROFILE, { 110, 135 }, KLV_1108_COMPRESSION_PROFILE_MAIN );
    add_src( KLV_1108_COMPRESSION_LEVEL, { 110, 135 }, std::string{ "5.1" } );
    add_src( KLV_1108_COMPRESSION_RATIO, { 110, 135 }, 22.0  );
    add_src( KLV_1108_STREAM_BITRATE, { 110, 135 }, uint64_t{ 26 } );
    add_src( KLV_1108_DOCUMENT_VERSION, { 110, 135 }, uint64_t{ 3 } );

    add_dst( KLV_1108_ASSESSMENT_POINT, { 110, 135 }, KLV_1108_ASSESSMENT_POINT_ARCHIVE );
    add_dst( KLV_1108_METRIC_LOCAL_SET, { 110, 135 },  klv_local_set{
    { KLV_1108_METRIC_SET_NAME, std::string{ "VNIIRS" } },
    { KLV_1108_METRIC_SET_VERSION, std::string{ "1.0" } },
    { KLV_1108_METRIC_SET_IMPLEMENTER, std::string{ "KWIVER" } },
    { KLV_1108_METRIC_SET_TIME, uint64_t{ 123456 } },
    { KLV_1108_METRIC_SET_VALUE, 5.0 }, } );
    add_dst( KLV_1108_COMPRESSION_TYPE, { 110, 135 }, KLV_1108_COMPRESSION_TYPE_H264 );
    add_dst( KLV_1108_COMPRESSION_PROFILE, { 110, 135 }, KLV_1108_COMPRESSION_PROFILE_MAIN );
    add_dst( KLV_1108_COMPRESSION_LEVEL, { 110, 135 }, std::string{ "5.1" } );
    add_dst( KLV_1108_COMPRESSION_RATIO, { 110, 135 }, 22.0  );
    add_dst( KLV_1108_STREAM_BITRATE, { 110, 135 }, uint64_t{ 26 } );
    add_dst( KLV_1108_DOCUMENT_VERSION, { 110, 135 }, uint64_t{ 3 } );
  }

  void add_src( klv_lds_key tag, typename klv_timeline::interval_t time_interval, klv_value const& value )
  {
    src_timeline.insert_or_find( standard, tag, index )->second.set( time_interval, value );
  }

  void add_dst( klv_lds_key tag, typename klv_timeline::interval_t time_interval, klv_value const& value )
  {
    dst_timeline.insert_or_find( standard, tag, index )->second.set( time_interval, value );
  }

  klv_timeline src_timeline;
  klv_timeline dst_timeline;
  klv_top_level_tag standard;
  klv_value index;
  std::vector< kv::timestamp > timestamps;
};

// ----------------------------------------------------------------------------
TEST_F ( klv_muxer_test, round_trip_buffered )
{
  // Turn timeline into packets
  klv_muxer muxer1( src_timeline );
  std::vector< klv_timed_packet > packets1;
  for( auto const& timestamp : timestamps )
  {
    muxer1.send_frame( timestamp.get_time_usec() );
  }
  for( auto const& timestamp : timestamps )
  {
    for( auto const packet : muxer1.receive_frame() )
    {
      packets1.emplace_back( klv_timed_packet{ packet, timestamp } );
    }
  }

  // Packets back into timeline
  klv_timeline new_timeline;
  klv_demuxer demuxer( new_timeline );
  for( auto const& packet : packets1 )
  {
    demuxer.send_frame( { packet.packet } );
  }

  // Compare timelines
  EXPECT_EQ( dst_timeline, new_timeline )
  << "\n" << dst_timeline << "\n\n" << new_timeline << "\n";

  // And timeline back into packets again
  klv_muxer muxer2( new_timeline );
  std::vector< klv_timed_packet > packets2;
  for( auto const& timestamp : timestamps )
  {
    muxer2.send_frame( timestamp.get_time_usec() );
  }
  for( auto const& timestamp : timestamps )
  {
    for( auto const packet : muxer2.receive_frame() )
    {
      packets2.emplace_back( klv_timed_packet{ packet, timestamp } );
    }
  }

  // Check that both sets of packets are the same
  EXPECT_EQ( packets1, packets2 );
}

// ----------------------------------------------------------------------------
TEST_F ( klv_muxer_test, round_trip_immediate )
{
  // Turn timeline into packets
  klv_muxer muxer1( src_timeline );
  std::vector< klv_timed_packet > packets1;
  for( auto const& timestamp : timestamps )
  {
    muxer1.send_frame( timestamp.get_time_usec() );
    for( auto const packet : muxer1.receive_frame() )
    {
      packets1.emplace_back( klv_timed_packet{ packet, timestamp } );
    }
  }

  // Packets back into timeline
  klv_timeline new_timeline;
  klv_demuxer demuxer( new_timeline );
  for( auto const& packet : packets1 )
  {
    demuxer.send_frame( { packet.packet } );
  }

  // Compare timelines
  EXPECT_EQ( dst_timeline, new_timeline )
  << "\n" << dst_timeline << "\n\n" << new_timeline << "\n";

  // And timeline back into packets again
  klv_muxer muxer2( new_timeline );
  std::vector< klv_timed_packet > packets2;
  for( auto const& timestamp : timestamps )
  {
    muxer2.send_frame( timestamp.get_time_usec() );
    for( auto const packet : muxer2.receive_frame() )
    {
      packets2.emplace_back( klv_timed_packet{ packet, timestamp } );
    }
  }

  // Check that both sets of packets are the same
  EXPECT_EQ( packets1, packets2 );
}
