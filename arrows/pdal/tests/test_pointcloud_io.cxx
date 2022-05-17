// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test PDAL pointcloud output

#include <test_gtest.h>
#include <test_tmpfn.h>

#include <arrows/pdal/algo/pointcloud_io.h>
#include <arrows/proj/geo_conv.h>
#include <vital/exceptions/io.h>
#include <vital/io/landmark_map_io.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <fstream>

namespace kv = kwiver::vital;

kv::path_t g_data_dir;

static std::string geo_origin_file = "pointcloud_data/geo_origin.txt";
static std::string landmarks_file = "pointcloud_data/landmarks.ply";
static std::string octahedron_base = "pointcloud_data/octahedron";
static std::string tmp_file = "pointcloud_data/pointcloud.las";

// ----------------------------------------------------------------------------

int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class pointcloud_io : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( pointcloud_io, create )
{
  kv::plugin_manager::instance().load_all_plugins();

  EXPECT_NE( nullptr, kv::algo::pointcloud_io::create( "pdal" ) );
}

TEST_F ( pointcloud_io, load )
{
  std::vector< kwiver::vital::vector_3d > points = {
    kwiver::vital::vector_3d( -1.0, 1.0, 0.0 ),
    kwiver::vital::vector_3d( -1.0, -1.0, 0.0 ),
    kwiver::vital::vector_3d( 1.0, -1.0, 0.0 ),
    kwiver::vital::vector_3d( 1.0, 1.0, 0.0 ),
    kwiver::vital::vector_3d( 0.0, 0.0, 0.7 ),
    kwiver::vital::vector_3d( 0.0, 0.0, -0.7 ) };

  std::vector< kwiver::vital::rgb_color > test_colors = {
    kwiver::vital::rgb_color( 255, 0, 0 ),
    kwiver::vital::rgb_color( 0, 255, 0 ),
    kwiver::vital::rgb_color( 0, 0, 255 ),
    kwiver::vital::rgb_color( 255, 0, 255 ),
    kwiver::vital::rgb_color( 255, 255, 255 ),
    kwiver::vital::rgb_color( 0, 0, 0 ) };

  auto pc_io = kwiver::arrows::pdal::pointcloud_io();

  for( std::string ext : { ".bpf", ".las", ".ply" } )
  {
    auto const octahedron_path = data_dir + "/" + octahedron_base + ext;

    auto pc_data = pc_io.load( octahedron_path );

    auto positions = pc_data.positions();
    auto colors = pc_data.colors();

    EXPECT_EQ( positions.size(), points.size() );

    for( unsigned int i = 0; i < positions.size(); ++i )
    {
      EXPECT_TRUE( positions[ i ].isApprox( points[ i ], 0.0000001 ) );
      EXPECT_EQ( colors[ i ], test_colors[ i ] );
    }
  }

  auto const octahedron_path = data_dir + "/" + octahedron_base + ".not";

  EXPECT_THROW( pc_io.load( octahedron_path ),
                kwiver::vital::invalid_file );
}

TEST_F ( pointcloud_io, save )
{
  auto const geo_origin_path = data_dir + "/" + geo_origin_file;
  auto const landmarks_path = data_dir + "/" + landmarks_file;
  auto const tmp_path = data_dir + "/" + tmp_file;
  std::ofstream ofs( tmp_path );
  ofs.close();

  // This will delete the temporary file even if an exception is thrown
  struct _tmp_file_deleter
  {
    ~_tmp_file_deleter()
    {
      std::remove( tmp_path.c_str() );
    }

    std::string tmp_path;
  } tmp_file_deleter{ tmp_path };

  kv::landmark_map_sptr landmark_map =
    kv::read_ply_file( landmarks_path );

  std::vector< kv::vector_3d > points;
  std::vector< kv::rgb_color > colors;
  points.reserve( landmark_map->size() );
  colors.reserve( landmark_map->size() );
  for( auto lm : landmark_map->landmarks() )
  {
    points.push_back( lm.second->loc() );
    colors.push_back( lm.second->color() );
  }

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  kv::set_geo_conv( &geo_conv );

  auto lgcs = kv::local_geo_cs();
  read_local_geo_cs_from_file( lgcs, geo_origin_path );

  auto pc_io = kwiver::arrows::pdal::pointcloud_io();
  pc_io.set_local_geo_cs( lgcs );
  pc_io.save( tmp_path, points, colors );
}

TEST_F ( pointcloud_io, save_landmarks )
{
  auto const geo_origin_path = data_dir + "/" + geo_origin_file;
  auto const landmarks_path = data_dir + "/" + landmarks_file;
  auto const tmp_path = data_dir + "/" + tmp_file;
  std::ofstream ofs( tmp_path );
  ofs.close();

  // This will delete the temporary file even if an exception is thrown
  struct _tmp_file_deleter
  {
    ~_tmp_file_deleter()
    {
      std::remove( tmp_path.c_str() );
    }

    std::string tmp_path;
  } tmp_file_deleter{ tmp_path };

  kv::landmark_map_sptr landmark_map =
    kv::read_ply_file( landmarks_path );

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  kv::set_geo_conv( &geo_conv );

  auto lgcs = kv::local_geo_cs();
  read_local_geo_cs_from_file( lgcs, geo_origin_path );

  auto pc_io = kwiver::arrows::pdal::pointcloud_io();
  pc_io.set_local_geo_cs( lgcs );
  pc_io.save( tmp_path, landmark_map );
}
