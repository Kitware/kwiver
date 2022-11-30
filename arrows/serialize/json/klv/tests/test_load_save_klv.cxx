// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/klv/klv_all.h>

#include <arrows/serialize/json/klv/load_save_klv.h>

#include <vital/internal/cereal/archives/json.hpp>
#include <vital/internal/cereal/types/vector.hpp>

#include <test_gtest.h>

#include <fstream>
#include <sstream>

#include <cfloat>

using namespace kwiver::arrows::klv;
namespace kv = kwiver::vital;
using kld = klv_lengthy< double >;

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
klv_local_set const test_0102_set = {
  { KLV_0102_SECURITY_CLASSIFICATION,
    KLV_0102_SECURITY_CLASSIFICATION_UNCLASSIFIED },
  { KLV_0102_COUNTRY_CODING_METHOD,
    KLV_0102_COUNTRY_CODING_METHOD_GENC_TWO_LETTER } };

// ---------------------------------------------------------------------------
klv_universal_set const test_0104_set = {
  { key_0104( KLV_0104_USER_DEFINED_TIMESTAMP ),
    uint64_t{ 4321 } },
  { key_0104( KLV_0104_EPISODE_NUMBER ),
    std::string{ "4.2" } },
  { key_0104( KLV_0104_DEVICE_DESIGNATION ),
    std::string{ "Bob" } } };

// ---------------------------------------------------------------------------
klv_local_set const test_0806_aoi_set = {
  { KLV_0806_AOI_SET_TYPE, KLV_0806_POI_AOI_TYPE_FRIENDLY } };

// ---------------------------------------------------------------------------
klv_local_set const test_0806_poi_set = {
  { KLV_0806_POI_SET_TYPE, KLV_0806_POI_AOI_TYPE_FRIENDLY } };

// ---------------------------------------------------------------------------
klv_local_set const test_0806_user_defined_set = {
  { KLV_0806_USER_DEFINED_SET_DATA_TYPE_ID,
    klv_0806_user_defined_data_type_id{
      KLV_0806_USER_DEFINED_SET_DATA_TYPE_UINT, 7 } },
  { KLV_0806_USER_DEFINED_SET_DATA, klv_0806_user_defined_data{ { 0xAB } } } };

// ---------------------------------------------------------------------------
klv_local_set const test_0806_set = {
  { KLV_0806_AOI_LOCAL_SET, test_0806_aoi_set },
  { KLV_0806_POI_LOCAL_SET, test_0806_poi_set },
  { KLV_0806_USER_DEFINED_LOCAL_SET, test_0806_user_defined_set } };

// ---------------------------------------------------------------------------
klv_local_set const test_0903_vtracker_set = {
  { KLV_0903_VTRACKER_VELOCITY,
    klv_0903_velocity_pack{ 1.0, 2.0, 3.0 } },
  { KLV_0903_VTRACKER_ACCELERATION,
    klv_0903_acceleration_pack{ 1.0, 2.0, 3.0 } } };

// ---------------------------------------------------------------------------
klv_local_set const test_0903_vtarget_set = {
  { KLV_0903_VTARGET_FPA_INDEX, klv_0903_fpa_index{ 1, 2 } },
  { KLV_0903_VTARGET_VMASK,
    klv_local_set{
      { KLV_0903_VMASK_POLYGON, std::vector< uint64_t >{ 1, 2, 3, 4 } },
      { KLV_0903_VMASK_BITMASK_SERIES,
        std::vector< klv_0903_pixel_run >{ { 1, 2 }, { 3, 4 } } } } },
  { KLV_0903_VTARGET_VOBJECT, klv_local_set{} },
  { KLV_0903_VTARGET_VFEATURE, klv_local_set{} },
  { KLV_0903_VTARGET_VTRACKER, test_0903_vtracker_set },
  { KLV_0903_VTARGET_VCHIP, klv_local_set{} },
  { KLV_0903_VTARGET_VCHIP_SERIES, std::vector< klv_local_set >{} },
  { KLV_0903_VTARGET_VOBJECT_SERIES, std::vector< klv_local_set >{} },
  { KLV_0903_VTARGET_LOCATION,
    klv_0903_location_pack{
      60.0, 30.0, 1000.0,
      klv_0903_sigma_pack{ 1.0, 2.0, 3.0 },
      klv_0903_rho_pack{ -1.0, 0.0, 1.0 } } } };

// ---------------------------------------------------------------------------
klv_local_set const test_0903_set = {
  { KLV_0903_VTARGET_SERIES,
    std::vector< klv_0903_vtarget_pack >{ { 1, test_0903_vtarget_set } } },
  { KLV_0903_ALGORITHM_SERIES, std::vector< klv_local_set >{} },
  { KLV_0903_ONTOLOGY_SERIES, std::vector< klv_local_set >{} }, };

// ---------------------------------------------------------------------------
klv_local_set const test_1202_set = {
  { KLV_1202_TRANSFORMATION_TYPE, KLV_1202_TRANSFORMATION_TYPE_OPTICAL } };

// ---------------------------------------------------------------------------
klv_local_set const test_1002_set = {
  { KLV_1002_RANGE_IMAGE_ENUMERATIONS,
    klv_1002_enumerations{
      KLV_1002_COMPRESSION_METHOD_NONE,
      KLV_1002_DATA_TYPE_DEPTH_RANGE_IMAGE,
      KLV_1002_SOURCE_RANGE_SENSOR } },
  { KLV_1002_SECTION_DATA_PACK,
    klv_1002_section_data_pack{
      2, 0,
      { { 2, 2 },
        { 100.0, 105.0, 95.0, 100.0 } },
      kv::nullopt,
      kld{ 1.0 },
      kld{ 2.0 },
      kv::nullopt } },
  { KLV_1002_GENERALIZED_TRANSFORMATION_LOCAL_SET, test_1202_set } };

// ---------------------------------------------------------------------------
klv_local_set const test_1206_set = {
  { KLV_1206_LOOK_DIRECTION, KLV_1206_LOOK_DIRECTION_LEFT },
  { KLV_1206_IMAGE_PLANE, KLV_1206_IMAGE_PLANE_GROUND }, };

// ---------------------------------------------------------------------------
klv_local_set const test_0601_set = {
  { KLV_0601_PRECISION_TIMESTAMP,
    uint64_t{ 1234 } },
  { KLV_0601_PLATFORM_HEADING_ANGLE, {} },
  { KLV_0601_PLATFORM_TRUE_AIRSPEED,
    kld{ 2.345, 1 } },
  { KLV_0601_MISSION_ID,
    std::string{ "TEST\0STRING", 11 } },
  { KLV_0601_IMAGE_HORIZON_PIXEL_PACK,
    klv_0601_image_horizon_pixel_pack{
      1, 2, 3, 4,
      klv_0601_image_horizon_locations{ 1.0, 2.0, 3.0, 4.0 } } },
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
  { KLV_0601_ACTIVE_PAYLOADS, std::set< uint16_t >{ 0, 1, 3 } },
  { KLV_0601_WEAPONS_STORES,
    std::vector< klv_0601_weapons_store >{
      { 0, 1, 2, 3,
        KLV_0601_WEAPON_GENERAL_STATUS_NO_STATUS,
        { KLV_0601_WEAPON_ENGAGEMENT_STATUS_BIT_FUSE_ENABLED,
          KLV_0601_WEAPON_ENGAGEMENT_STATUS_BIT_LASER_ENABLED },
        "Water Balloon" },
      { 4, 5, 6, 7,
        KLV_0601_WEAPON_GENERAL_STATUS_OFF,
        {},
        "Squirt Gun" }, } },
  { KLV_0601_GENERIC_FLAG_DATA,
    std::set< klv_0601_generic_flag_data_bit >{
      KLV_0601_GENERIC_FLAG_DATA_BIT_AUTO_TRACK,
      KLV_0601_GENERIC_FLAG_DATA_BIT_ICING_STATUS } },
  { KLV_0601_POSITIONING_METHOD_SOURCE,
    std::set< klv_0601_positioning_method_source_bit >{
      KLV_0601_POSITIONING_METHOD_SOURCE_BIT_ON_BOARD_INS,
      KLV_0601_POSITIONING_METHOD_SOURCE_BIT_GPS } },
  { KLV_0601_SENSOR_FOV_NAME,
    KLV_0601_SENSOR_FOV_NAME_MEDIUM },
  { KLV_0601_AIRBASE_LOCATIONS,
    klv_0601_airbase_locations{
      klv_0601_location_dlp{ 1.0, 2.0, 3.0 },
      klv_0601_location_dlp{ 4.0, 5.0 } } },
  { KLV_0601_COUNTRY_CODES,
    klv_0601_country_codes{
      KLV_0102_COUNTRY_CODING_METHOD_GENC_THREE_LETTER,
      std::string{ "USA" },
      kv::nullopt,
      kv::nullopt } },
  { KLV_0601_PAYLOAD_LIST,
    std::vector< klv_0601_payload_record >{
      { 2, KLV_0601_PAYLOAD_TYPE_ELECTRO_OPTICAL, "Camera" } } },
  { KLV_0601_WAVELENGTHS_LIST,
    std::vector< klv_0601_wavelength_record >{
      { 7, 13.0, 14.0, "Wavelength" } } },
  { KLV_0601_WAYPOINT_LIST,
    std::vector< klv_0601_waypoint_record >{
      { 1, -3,
        std::set< klv_0601_waypoint_info_bit >{ KLV_0601_WAYPOINT_INFO_BIT_MODE },
        kv::nullopt } } },
  { KLV_0601_MIIS_CORE_IDENTIFIER,
    klv_1204_miis_id{
      2,
      KLV_1204_DEVICE_ID_TYPE_PHYSICAL,
      KLV_1204_DEVICE_ID_TYPE_NONE,
      klv_uuid{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F } } },
  { KLV_0601_VIEW_DOMAIN,
    klv_0601_view_domain{
      klv_0601_view_domain_interval{ 30.0, 60.0 },
      kv::nullopt,
      kv::nullopt } },
  { KLV_0601_RVT_LOCAL_SET, test_0806_set },
  { KLV_0601_VMTI_LOCAL_SET, test_0903_set },
  { KLV_0601_RANGE_IMAGE_LOCAL_SET, test_1002_set },
  { KLV_0601_SDCC_FLP,
    klv_1010_sdcc_flp{
      { KLV_0601_SENSOR_LATITUDE, KLV_0601_SENSOR_LONGITUDE },
      { 4.0, 2.1e-64 },
      { 0.5 },
      4, 3,
      false, true,
      true,
      false } },
  { KLV_0601_MISSION_ID, klv_blob{ 0x00, 0xFF } } };

// ---------------------------------------------------------------------------
klv_local_set const test_1107_set = {
  { KLV_1107_SLANT_RANGE_PEDIGREE,
    KLV_1107_SLANT_RANGE_PEDIGREE_CALCULATED }
};

// ---------------------------------------------------------------------------
klv_local_set const test_1108_metric_set = {
  { KLV_1108_METRIC_SET_NAME,
    std::string{ "VNIIRS" } },
  { KLV_1108_METRIC_SET_IMPLEMENTER,
    klv_1108_metric_implementer{ "Kitware, Inc.",
                                 "Computer Vision" } },
  { KLV_1108_METRIC_SET_TIME,
    uint64_t{ 1357924680 } },
  { KLV_1108_METRIC_SET_VALUE,
    kld{ -std::numeric_limits< double >::infinity(), 8 } } };

// ---------------------------------------------------------------------------
klv_local_set const test_1108_set = {
  { KLV_1108_ASSESSMENT_POINT, KLV_1108_ASSESSMENT_POINT_ARCHIVE },
  { KLV_1108_METRIC_PERIOD_PACK,
    klv_1108_metric_period_pack{ 100, 100 } },
  { KLV_1108_WINDOW_CORNERS_PACK,
    klv_1108_window_corners_pack{ { 64, 128, 256, 512 } } },
  { KLV_1108_METRIC_LOCAL_SET, test_1108_metric_set },
  { KLV_1108_COMPRESSION_TYPE, KLV_1108_COMPRESSION_TYPE_H262 },
  { KLV_1108_COMPRESSION_PROFILE, KLV_1108_COMPRESSION_PROFILE_HIGH },
  { KLV_1108_COMPRESSION_LEVEL, klv_value{} } };

// ---------------------------------------------------------------------------
std::vector< klv_timed_packet > const test_packets = {
  { { klv_0102_key(), test_0102_set }, kv::timestamp{ 0, 0 } },
  { { klv_0104_key(), test_0104_set }, kv::timestamp{} },
  { { klv_0601_key(), test_0601_set }, kv::timestamp{ 1024, 7 } },
  { { klv_1107_key(), test_1107_set }, kv::timestamp{} },
  { { klv_1108_key(), test_1108_set }, kv::timestamp{ 2048, 8 } } };

// ----------------------------------------------------------------------------
auto const options = cereal::JSONOutputArchive::Options{
  cereal::JSONOutputArchive::Options::MaxPrecision(),
  cereal::JSONOutputArchive::Options::IndentChar::tab, 1 };

// ----------------------------------------------------------------------------
TEST_F ( load_save_klv, round_trip )
{
  std::stringstream ss;
  {
    cereal::JSONOutputArchive archive( ss, options );
    cereal::save( archive, test_packets );
  }

  std::vector< klv_timed_packet > result_packets;
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
    cereal::JSONOutputArchive archive( ss, options );
    cereal::save( archive, test_packets );
  }
  ss << '\n';

  EXPECT_EQ( golden_string, ss.str() );
}
