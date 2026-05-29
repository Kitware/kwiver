// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Tests for core mesh texturing algorithm.

#include <test_gtest.h>

#include <arrows/core/algo/texture_mesh.h>
#include <arrows/core/algo/uv_unwrap_mesh.h>
#include <arrows/core/mesh_operations.h>

#include <vital/algo/image_io.h>
#include <vital/algo/texture_mesh.h>
#include <vital/exceptions/base.h>
#include <vital/io/camera_io.h>
#include <vital/io/mesh_io.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/camera_intrinsics.h>
#include <vital/types/camera_perspective.h>
#include <vital/types/image_container.h>
#include <vital/types/mesh.h>
#include <vital/types/mesh_container.h>
#include <vital/types/rotation.h>
#include <vital/util/transform_image.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;

kwiver::vital::path_t g_data_dir;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();
  GET_ARG( 1, g_data_dir );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
// Build a flat unit square mesh in the XY plane at Z=0, made of two triangles.
// A=(-1,-1,0), B=(1,-1,0), C=(-1,1,0), D=(1,1,0)
// Face normals point in +Z direction, visible from a camera at Z=10.
mesh_container_sptr
make_square_mesh()
{
  std::vector< vector_3d > verts = {
    { -1, -1, 0 }, { 1, -1, 0 }, { -1, 1, 0 }, { 1, 1, 0 } };

  std::vector< mesh_regular_face< 3 > > faces = {
    mesh_regular_face< 3 >( { 0, 1, 2 } ),
    mesh_regular_face< 3 >( { 1, 3, 2 } ) };

  std::unique_ptr< mesh_vertex_array_base > verts_ptr(
    new mesh_vertex_array< 3 >( verts ) );
  std::unique_ptr< mesh_face_array_base > faces_ptr(
    new mesh_regular_face_array< 3 >( faces ) );

  auto m = kwiver::vital::mesh(
    std::move( verts_ptr ),
    std::move( faces_ptr ) );
  auto cont = std::make_shared< simple_mesh_container >( m );

  // UV unwrap so texture coordinates are set and valid
  kwiver::arrows::core::uv_unwrap_mesh unwrap;
  unwrap.unwrap( cont );

  return cont;
}

// ----------------------------------------------------------------------------
// Camera at (0,0,10) looking toward -Z, 1000x1000 image, focal=780.
// Same configuration as test_render_mesh_depth_map.
camera_perspective_sptr
make_camera()
{
  camera_intrinsics_sptr intrinsics( new simple_camera_intrinsics(
    780, { 500, 500 }, 1.0, 0.0, {}, 1000, 1000 ) );

  matrix_3x3d rot = matrix_3x3d::Zero();
  rot( 0, 0 ) = 1.0;
  rot( 1, 1 ) = -1.0;
  rot( 2, 2 ) = -1.0;

  rotation_d orientation( rot );

  return camera_perspective_sptr(
    new simple_camera_perspective(
      { 0, 0, 10 }, orientation.inverse(), intrinsics ) );
}

// ----------------------------------------------------------------------------
// Create a solid-color 3-channel uint8 frame image of given size.
image_container_sptr
make_frame(
  uint8_t r, uint8_t g, uint8_t b,
  size_t w = 1000, size_t h = 1000 )
{
  image_of< uint8_t > img( w, h, 3 );
  for( size_t x = 0; x < w; ++x )
  {
    for( size_t y = 0; y < h; ++y )
    {
      img( x, y, 0 ) = r;
      img( x, y, 1 ) = g;
      img( x, y, 2 ) = b;
    }
  }
  return std::make_shared< simple_image_container >( img );
}

// ----------------------------------------------------------------------------
// Create a zero-initialized RGBA uint8 output texture of given side length.
image_container_sptr
make_output_uint8( size_t size = 256 )
{
  image_of< uint8_t > img( size, size, 4 );
  transform_image( img, []( uint8_t ){ return uint8_t( 0 ); } );
  return std::make_shared< simple_image_container >( img );
}

// ----------------------------------------------------------------------------
// Create a zero-initialized RGBA float output texture (for texture_xyz).
image_container_sptr
make_output_float( size_t size = 256 )
{
  image_of< float > img( size, size, 4 );
  transform_image( img, []( float ){ return 0.0f; } );
  return std::make_shared< simple_image_container >( img );
}

// ----------------------------------------------------------------------------
// Count pixels with alpha>0 and verify they all match the expected RGB color.
// Returns {colored_pixel_count, all_colors_match}.
std::pair< int, bool >
check_rgba_output(
  image_container_sptr const& output,
  uint8_t expected_r, uint8_t expected_g, uint8_t expected_b )
{
  int colored = 0;
  bool ok = true;
  image_of< uint8_t > img( output->get_image() );
  for( size_t x = 0; x < output->width(); ++x )
  {
    for( size_t y = 0; y < output->height(); ++y )
    {
      if( img( x, y, 3 ) > 0 )
      {
        ++colored;
        if( img( x, y, 0 ) != expected_r ||
            img( x, y, 1 ) != expected_g ||
            img( x, y, 2 ) != expected_b ||
            img( x, y, 3 ) != 255 )
        {
          ok = false;
        }
      }
    }
  }
  return { colored, ok };
}

// ----------------------------------------------------------------------------
TEST ( texture_mesh, create )
{
  plugin_manager::instance().load_all_plugins();
  EXPECT_NE(
    nullptr,
    create_algorithm< algo::texture_mesh >( "core" ) );
}

// ----------------------------------------------------------------------------
class texture_mesh_test : public ::testing::Test
{
public:
  void
  SetUp() override
  {
    mesh_cont = make_square_mesh();
    camera    = make_camera();
  }

  mesh_container_sptr mesh_cont;
  camera_perspective_sptr camera;
};

// ----------------------------------------------------------------------------
// texture() should paint all covered pixels with the solid frame color.
TEST_F ( texture_mesh_test, texture_single_frame )
{
  auto frame  = make_frame( 200, 100, 50 );
  auto output = make_output_uint8();

  kwiver::arrows::core::texture_mesh texer;
  texer.texture( mesh_cont, output, frame, camera );

  auto [ colored, ok ] = check_rgba_output( output, 200, 100, 50 );
  EXPECT_GT( colored, 0 ) << "No pixels were textured";
  EXPECT_TRUE( ok ) << "Textured pixels have unexpected color or alpha";
}

// ----------------------------------------------------------------------------
// texture_xyz() should write mesh XYZ coordinates into the float output image.
// Our mesh lies in the Z=0 plane, so the Z channel of every filled pixel
// should be near 0.
TEST_F ( texture_mesh_test, texture_xyz_z_values )
{
  auto output = make_output_float();

  kwiver::arrows::core::texture_mesh texer;
  texer.texture_xyz( mesh_cont, output );

  image_of< float > result( output->get_image() );
  int filled = 0;
  bool z_ok  = true;
  for( size_t x = 0; x < output->width(); ++x )
  {
    for( size_t y = 0; y < output->height(); ++y )
    {
      if( result( x, y, 3 ) > 0.0f )
      {
        ++filled;
        if( std::abs( result( x, y, 2 ) ) > 0.01f )
        {
          z_ok = false;
        }
      }
    }
  }
  EXPECT_GT( filled, 0 ) << "No pixels were filled by texture_xyz";
  EXPECT_TRUE( z_ok )
    << "Z channel of filled pixels should be near 0 (mesh Z=0 plane)";
}

// ----------------------------------------------------------------------------
// alpha channel of filled pixels should be 1.0 (not 0, not partial).
TEST_F ( texture_mesh_test, texture_xyz_alpha )
{
  auto output = make_output_float();

  kwiver::arrows::core::texture_mesh texer;
  texer.texture_xyz( mesh_cont, output );

  image_of< float > result( output->get_image() );
  bool alpha_ok = true;
  for( size_t x = 0; x < output->width(); ++x )
  {
    for( size_t y = 0; y < output->height(); ++y )
    {
      float a = result( x, y, 3 );
      if( a != 0.0f && a != 1.0f )
      {
        alpha_ok = false;
      }
    }
  }
  EXPECT_TRUE( alpha_ok )
    << "Alpha channel in texture_xyz output should be exactly 0 or 1";
}

// ----------------------------------------------------------------------------
// texture_list "all" mode: each output image is independently textured with
// its corresponding frame.
TEST_F ( texture_mesh_test, texture_list_all_mode )
{
  auto frame_red  = make_frame( 255, 0, 0 );
  auto frame_blue = make_frame( 0, 0, 255 );

  image_container_sptr_list frames  = { frame_red, frame_blue };
  camera_sptr_list cameras = { camera, camera };
  image_container_sptr_list outputs = { make_output_uint8(),
                                        make_output_uint8() };

  kwiver::arrows::core::texture_mesh texer;
  texer.texture_list( mesh_cont, outputs, frames, cameras, "all" );

  auto [ red_count, red_ok ]   = check_rgba_output( outputs[ 0 ], 255, 0, 0 );
  auto [ blue_count, blue_ok ] = check_rgba_output( outputs[ 1 ], 0, 0, 255 );

  EXPECT_GT( red_count, 0 ) << "Output 0 should have textured pixels";
  EXPECT_TRUE( red_ok ) << "Output 0 should be solid red";
  EXPECT_GT( blue_count, 0 ) << "Output 1 should have textured pixels";
  EXPECT_TRUE( blue_ok ) << "Output 1 should be solid blue";
}

// ----------------------------------------------------------------------------
// texture_list "mean" mode: pixels should be the average of the two frame
// colors. Mean of R=200 and R=0 is 100; mean of B=0 and B=200 is 100.
TEST_F ( texture_mesh_test, texture_list_mean_mode )
{
  auto frame_red  = make_frame( 200, 0, 0 );
  auto frame_blue = make_frame( 0, 0, 200 );

  image_container_sptr_list frames  = { frame_red, frame_blue };
  camera_sptr_list cameras = { camera, camera };
  image_container_sptr_list outputs = { make_output_uint8() };

  kwiver::arrows::core::texture_mesh texer;
  texer.texture_list( mesh_cont, outputs, frames, cameras, "mean" );

  image_of< uint8_t > result( outputs[ 0 ]->get_image() );
  int filled = 0;
  bool ok    = true;
  for( size_t x = 0; x < outputs[ 0 ]->width(); ++x )
  {
    for( size_t y = 0; y < outputs[ 0 ]->height(); ++y )
    {
      if( result( x, y, 3 ) > 0 )
      {
        ++filled;
        // Allow ±1 for integer rounding
        if( std::abs( ( int ) result( x, y, 0 ) - 100 ) > 1 ||
            result( x, y, 1 ) != 0 ||
            std::abs( ( int ) result( x, y, 2 ) - 100 ) > 1 )
        {
          ok = false;
        }
      }
    }
  }
  EXPECT_GT( filled, 0 ) << "Mean output should have textured pixels";
  EXPECT_TRUE( ok ) << "Mean color should be approximately (100, 0, 100)";
}

// ----------------------------------------------------------------------------
// texture_list "median" mode: with 3 frames (black, red, white), the median
// of each channel should be the red frame's value.
TEST_F ( texture_mesh_test, texture_list_median_mode )
{
  image_container_sptr_list frames  = {
    make_frame( 0, 0, 0 ),       // black
    make_frame( 200, 0, 0 ),     // red
    make_frame( 255, 255, 255 )  // white
  };
  camera_sptr_list cameras = { camera, camera, camera };
  image_container_sptr_list outputs = { make_output_uint8() };

  kwiver::arrows::core::texture_mesh texer;
  texer.texture_list( mesh_cont, outputs, frames, cameras, "median" );

  image_of< uint8_t > result( outputs[ 0 ]->get_image() );
  int filled = 0;
  bool ok    = true;
  for( size_t x = 0; x < outputs[ 0 ]->width(); ++x )
  {
    for( size_t y = 0; y < outputs[ 0 ]->height(); ++y )
    {
      if( result( x, y, 3 ) > 0 )
      {
        ++filled;
        // Median of {0, 200, 255} = 200 for R; {0, 0, 255} = 0 for G;
        // {0, 0, 255} = 0 for B
        if( result( x, y, 0 ) != 200 ||
            result( x, y, 1 ) != 0   ||
            result( x, y, 2 ) != 0 )
        {
          ok = false;
        }
      }
    }
  }
  EXPECT_GT( filled, 0 ) << "Median output should have textured pixels";
  EXPECT_TRUE( ok ) << "Median color should be (200, 0, 0)";
}

// ----------------------------------------------------------------------------
// NULL inputs to texture() should throw invalid_value.
TEST_F ( texture_mesh_test, texture_null_mesh_throws )
{
  auto frame  = make_frame( 200, 100, 50 );
  auto output = make_output_uint8();
  kwiver::arrows::core::texture_mesh texer;
  EXPECT_THROW(
    texer.texture( nullptr, output, frame, camera ),
    invalid_value );
}

TEST_F ( texture_mesh_test, texture_null_output_throws )
{
  auto frame = make_frame( 200, 100, 50 );
  kwiver::arrows::core::texture_mesh texer;
  EXPECT_THROW(
    texer.texture( mesh_cont, nullptr, frame, camera ),
    invalid_value );
}

TEST_F ( texture_mesh_test, texture_null_frame_throws )
{
  auto output = make_output_uint8();
  kwiver::arrows::core::texture_mesh texer;
  EXPECT_THROW(
    texer.texture( mesh_cont, output, nullptr, camera ),
    invalid_value );
}

TEST_F ( texture_mesh_test, texture_null_camera_throws )
{
  auto frame  = make_frame( 200, 100, 50 );
  auto output = make_output_uint8();
  kwiver::arrows::core::texture_mesh texer;
  EXPECT_THROW(
    texer.texture( mesh_cont, output, frame, nullptr ),
    invalid_value );
}

// ----------------------------------------------------------------------------
// Invalid mode string to texture_list() should throw.
TEST_F ( texture_mesh_test, texture_list_invalid_mode_throws )
{
  image_container_sptr_list frames  = { make_frame( 200, 100, 50 ) };
  camera_sptr_list cameras = { camera };
  image_container_sptr_list outputs = { make_output_uint8() };
  kwiver::arrows::core::texture_mesh texer;
  EXPECT_THROW(
    texer.texture_list( mesh_cont, outputs, frames, cameras, "bogus" ),
    invalid_value );
}

// ----------------------------------------------------------------------------
// Non-RGBA output image (3 channels) should throw in configure().
TEST_F ( texture_mesh_test, texture_non_rgba_output_throws )
{
  auto frame = make_frame( 200, 100, 50 );
  // 3-channel output — should fail the depth!=4 check
  image_of< uint8_t > bad_img( 256, 256, 3 );
  transform_image( bad_img, []( uint8_t ){ return uint8_t( 0 ); } );

  auto bad_output = std::make_shared< simple_image_container >( bad_img );

  kwiver::arrows::core::texture_mesh texer;
  EXPECT_THROW(
    texer.texture( mesh_cont, bad_output, frame, camera ),
    invalid_value );
}

// ----------------------------------------------------------------------------
// Data-driven plausibility test using a synthetic cube mesh and 6 axis-aligned
// cameras, each holding a distinct solid color.  The pre-UV-unwrapped mesh and
// KRTD camera files live in test_data/texturing_test_data/.
//
// We verify semantics rather than pixel-exact values so the test survives
// changes to the UV packing algorithm: each of the 6 expected colors must
// appear with roughly equal coverage and no unexpected colors should appear.
TEST ( texture_mesh_cube, six_face_colors )
{
  if( g_data_dir.empty() )
  {
    std::cout << "[ SKIPPED ] No test data directory provided\n";
    return;
  }

  plugin_manager::instance().load_all_plugins();

  const std::string data_dir = g_data_dir + "/texturing_test_data";

  // Load the pre-UV-unwrapped cube mesh and ensure it is triangulated.
  auto mesh_sptr = read_mesh( data_dir + "/cube_unwrapped.obj" );
  ASSERT_NE( nullptr, mesh_sptr );
  kwiver::arrows::core::mesh_triangulate( *mesh_sptr );

  auto mesh_cont = std::make_shared< simple_mesh_container >( *mesh_sptr );
  ASSERT_NE( mesh::TEX_COORD_NONE, mesh_cont->has_tex_coords() );

  // Each camera name maps to the solid RGB color that camera's frame holds.
  // Colors match the Python generation script.
  struct CamColor { std::string name; uint8_t r, g, b; };

  const std::vector< CamColor > cam_colors = {
    { "neg_z", 255, 255,   0 },  // yellow
    { "pos_z",   0,   0, 255 },  // blue
    { "neg_y", 255,   0, 255 },  // magenta
    { "pos_y",   0, 255,   0 },  // green
    { "neg_x",   0, 255, 255 },  // cyan
    { "pos_x", 255,   0,   0 },  // red
  };

  const std::string krtd_dir = data_dir + "/krtd";
  const size_t img_size = 512;

  image_container_sptr_list frames;
  camera_sptr_list cameras;

  for( auto const& cc : cam_colors )
  {
    // Solid-color frame.
    image_of< uint8_t > img( img_size, img_size, 3 );
    for( size_t x = 0; x < img_size; ++x )
    {
      for( size_t y = 0; y < img_size; ++y )
      {
        img( x, y, 0 ) = cc.r;
        img( x, y, 1 ) = cc.g;
        img( x, y, 2 ) = cc.b;
      }
    }
    frames.push_back( std::make_shared< simple_image_container >( img ) );

    // Camera from KRTD; set image dimensions to match frame.
    auto cam = read_krtd_file( cc.name, krtd_dir );
    auto persp = std::dynamic_pointer_cast< simple_camera_perspective >( cam );
    ASSERT_NE( nullptr, persp );

    auto intr =
      std::make_shared< simple_camera_intrinsics >( *persp->intrinsics() );
    intr->set_image_width( img_size );
    intr->set_image_height( img_size );
    persp->set_intrinsics( intr );
    cameras.push_back( cam );
  }

  // Pre-allocate a single RGBA output and run in mean mode.
  const size_t tex_size = 512;
  image_of< uint8_t > out_img( tex_size, tex_size, 4 );
  transform_image( out_img, []( uint8_t ){ return uint8_t( 0 ); } );

  image_container_sptr_list outputs = {
    std::make_shared< simple_image_container >( out_img ) };

  kwiver::arrows::core::texture_mesh texer;
  texer.texture_list( mesh_cont, outputs, frames, cameras, "mean" );

  // Uncomment to inspect the output texture:
  // auto io =
  //  kwiver::vital::create_algorithm< kwiver::vital::algo::image_io >( "ocv" );
  // io->save( "/tmp/cube_texture_debug.png", outputs[ 0 ] );

  // Count pixels per color among filled (alpha > 0) pixels.
  image_of< uint8_t > result( outputs[ 0 ]->get_image() );
  std::map< std::tuple< uint8_t, uint8_t, uint8_t >, int > color_counts;
  int total_filled = 0;

  for( size_t x = 0; x < tex_size; ++x )
  {
    for( size_t y = 0; y < tex_size; ++y )
    {
      if( result( x, y, 3 ) == 0 ) { continue; }
      ++total_filled;

      auto key = std::make_tuple(
        result( x, y, 0 ),
        result( x, y, 1 ),
        result( x, y, 2 ) );
      ++color_counts[ key ];
    }
  }

  // The atlas should be substantially filled — cube has 6 faces so at least
  // 50% of a well-packed 512×512 atlas should be covered.
  EXPECT_GT( total_filled, ( int ) ( tex_size * tex_size * 0.5 ) )
    << "Atlas is less than 50% filled";

  // Every expected color must be present.
  EXPECT_EQ( ( int ) cam_colors.size(), ( int ) color_counts.size() )
    << "Expected exactly 6 distinct colors in the atlas";

  // Each face should cover roughly 1/6 of filled pixels — allow ±50% variance.
  int expected_per_face = total_filled / ( int ) cam_colors.size();
  for( auto const& cc : cam_colors )
  {
    auto key = std::make_tuple( cc.r, cc.g, cc.b );
    auto it  = color_counts.find( key );
    EXPECT_NE( color_counts.end(), it )
      << "Color (" << ( int ) cc.r << "," << ( int ) cc.g << "," << ( int ) cc.b
      << ") for camera " << cc.name << " not found in atlas";
    if( it != color_counts.end() )
    {
      EXPECT_GT( it->second, expected_per_face / 2 )
        << "Camera " << cc.name << " covers too few pixels";
      EXPECT_LT( it->second, expected_per_face * 2 )
        << "Camera " << cc.name << " covers too many pixels";
    }
  }
}
