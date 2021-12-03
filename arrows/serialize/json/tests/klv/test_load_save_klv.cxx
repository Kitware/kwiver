// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/klv/klv_0104_new.h>
#include <arrows/klv/klv_0601_new.h>
#include <arrows/klv/klv_1108.h>
#include <arrows/klv/klv_1108_metric_set.h>
#include <arrows/serialize/json/klv/load_save_klv.h>
#include <vital/internal/cereal/archives/json.hpp>

#include <test_gtest.h>

#include <fstream>
#include <sstream>

using namespace kwiver::arrows::klv;

kwiver::vital::path_t g_data_dir;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ---------------------------------------------------------------------------
class load_save_klv : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ---------------------------------------------------------------------------
klv_uds_key
key_0104( klv_0104_tag tag )
{
  return klv_0104_traits_lookup().by_tag( tag ).uds_key();
}

// ---------------------------------------------------------------------------
std::vector< klv_packet > const test_packets = {
  { klv_0104_key(), klv_universal_set{
      { key_0104( KLV_0104_USER_DEFINED_TIMESTAMP ),
        uint64_t{ 4321 } },
      { key_0104( KLV_0104_EPISODE_NUMBER ),
        4.2 },
      { key_0104( KLV_0104_DEVICE_DESIGNATION ),
        std::string{ "Bob" } } } },
  { klv_0601_key(),
    klv_local_set{
      { KLV_0601_PRECISION_TIMESTAMP,
        uint64_t{ 1234 } },
      { KLV_0601_PLATFORM_TRUE_AIRSPEED,
        2.345 },
      { KLV_0601_MISSION_ID,
        std::string{ "TEST STRING" } },
      { KLV_0601_IMAGE_HORIZON_PIXEL_PACK,
        klv_blob{ { 0x00, 0x24, 0x38, 0x00 } } },
      { KLV_0601_CONTROL_COMMAND,
        klv_0601_control_command{ 0, "command!", 0 } },
      { KLV_0601_SENSOR_FRAME_RATE_PACK,
        klv_0601_frame_rate{ 30, 1 } },
      { KLV_0601_ICING_DETECTED,
        KLV_0601_ICING_DETECTED_FALSE },
      { KLV_0601_OPERATIONAL_MODE,
        KLV_0601_OPERATIONAL_MODE_TEST },
      { KLV_0601_PLATFORM_STATUS,
        KLV_0601_PLATFORM_STATUS_ACTIVE },
      { KLV_0601_SENSOR_CONTROL_MODE,
        KLV_0601_SENSOR_CONTROL_MODE_OFF },
      { KLV_0601_SENSOR_FOV_NAME,
        KLV_0601_SENSOR_FOV_NAME_MEDIUM }, } },
  { klv_1108_key(), klv_local_set{
      { KLV_1108_ASSESSMENT_POINT,
        KLV_1108_ASSESSMENT_POINT_ARCHIVE },
      { KLV_1108_METRIC_PERIOD_PACK,
        klv_1108_metric_period_pack{ 100, 100 } },
      { KLV_1108_WINDOW_CORNERS_PACK,
        klv_1108_window_corners_pack{ { 64, 128, 256, 512 } } },
      { KLV_1108_METRIC_LOCAL_SET,
        klv_local_set{
          { KLV_1108_METRIC_SET_NAME,
            std::string{ "VNIIRS" } },
          { KLV_1108_METRIC_SET_IMPLEMENTER,
            klv_1108_metric_implementer{ "Kitware, Inc.",
                                         "Computer Vision" } },
          { KLV_1108_METRIC_SET_TIME,
            uint64_t{ 1357924680 } },
          { KLV_1108_METRIC_SET_VALUE,
            -std::numeric_limits< double >::infinity() } } },
      { KLV_1108_COMPRESSION_TYPE,
        KLV_1108_COMPRESSION_TYPE_H262 },
      { KLV_1108_COMPRESSION_PROFILE,
        KLV_1108_COMPRESSION_PROFILE_HIGH },
      { KLV_1108_COMPRESSION_LEVEL,
        klv_value{} } } }, };

// ----------------------------------------------------------------------------
TEST_F ( load_save_klv, round_trip )
{
  std::stringstream ss;
  {
    cereal::JSONOutputArchive archive( ss );
    cereal::save( archive, test_packets );
  }

  std::vector< klv_packet > result_packets;
  {
    cereal::JSONInputArchive archive( ss );
    cereal::load( archive, result_packets );
  }

  EXPECT_EQ( test_packets, result_packets );
}

// ----------------------------------------------------------------------------
TEST_F ( load_save_klv, compare_golden )
{
  std::string golden_string;
  {
    std::ifstream fs;
    fs.open( data_dir + "/klv_gold.json" );
    std::stringstream ss;
    ss << fs.rdbuf();
    golden_string = ss.str();
  }

  std::stringstream ss;
  {
    cereal::JSONOutputArchive archive( ss );
    cereal::save( archive, test_packets );
  }
  ss << '\n';

  EXPECT_EQ( golden_string, ss.str() );
}
