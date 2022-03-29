// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test core mesh_io functionality

#include <kwiversys/SystemTools.hxx>
#include <tests/test_gtest.h>
#include <vital/exceptions.h>

#include <vital/io/mesh_io.cxx>

#include <gtest/gtest.h>

kwiver::vital::path_t g_data_dir;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  GET_ARG(1, g_data_dir);
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class mesh_io : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(mesh_io, invalid_output_file)
{
  EXPECT_NO_THROW( kwiver::vital::check_output_file( "temp/name" ) );
  EXPECT_THROW( kwiver::vital::check_output_file( "temp" ),
    kwiver::vital::file_write_exception );
  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, invalid_input_file)
{
  EXPECT_THROW( kwiver::vital::check_input_file( data_dir + "/nonexistant" ),
    kwiver::vital::file_not_found_exception );
  EXPECT_THROW( kwiver::vital::check_input_file( data_dir + "/videos" ),
    kwiver::vital::file_not_found_exception );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_invalid_type)
{
  kwiver::vital::mesh_sptr empty_read_mesh = kwiver::vital::read_mesh(
    data_dir + "/pipeline_data/geo_origin.txt");

  EXPECT_EQ( empty_read_mesh, nullptr );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_write_ply2)
{
  std::string original =
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.ply2";
  std::string copy = "temp/aphill_240_1fps_crf32_sm_fused_mesh2.ply2";

  kwiver::vital::mesh_sptr mesh_ply2 = kwiver::vital::read_ply2( original );
  kwiver::vital::write_ply2( copy, *mesh_ply2 );

  std::ifstream original_stream(original.c_str());
  std::ifstream copy_stream(copy.c_str());

  EXPECT_EQ( original_stream.gcount(), copy_stream.gcount() );

  std::string original_str = "";
  std::string copy_str = "";
  while( original_stream >> original_str && copy_stream >> copy_str ){
    EXPECT_EQ( original_str, copy_str );
  }

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_write_obj)
{
  std::string original =
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.obj";
  std::string copy = "temp/aphill_240_1fps_crf32_sm_fused_mesh2.obj";

  kwiver::vital::mesh_sptr mesh_obj = kwiver::vital::read_obj( original );
  kwiver::vital::write_obj( copy, *mesh_obj );

  std::ifstream original_stream(original.c_str());
  std::ifstream copy_stream(copy.c_str());

  EXPECT_EQ( original_stream.gcount(), copy_stream.gcount() );

  std::string original_str = "";
  std::string copy_str = "";
  while( original_stream >> original_str && copy_stream >> copy_str ){
    EXPECT_EQ( original_str, copy_str );
  }

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}
