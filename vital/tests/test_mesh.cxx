// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test core mesh functionality

#include <kwiversys/SystemTools.hxx>
#include <tests/test_gtest.h>
#include <vital/exceptions.h>

#include <vital/io/mesh_io.h>

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
class mesh : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(mesh, group_names)
{
  kwiver::vital::mesh_sptr mesh_ply = kwiver::vital::read_mesh(
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.ply");

  std::shared_ptr<kwiver::vital::mesh_face_array> faces =
    std::make_shared<kwiver::vital::mesh_face_array>(mesh_ply->faces());

  for(unsigned int i=0; i<faces->size(); i++){
    EXPECT_EQ( faces->group_name(i), "" );
  }

  EXPECT_EQ( faces->make_group( "testing name" ), faces->size() );
  EXPECT_EQ( faces->make_group( "second testing name" ), 0 );

  for(unsigned int i=0; i<faces->size(); i++){
    EXPECT_EQ( faces->group_name(i), "testing name" );
  }

  EXPECT_EQ( faces->group_name( faces->size()+1 ), "" );

  std::set<unsigned int> faces_set = faces->group_face_set( "testing name" );
  for(unsigned int i=0; i<faces->size(); i++){
    EXPECT_NE( faces_set.find( i ), faces_set.end() );
  }
}
