// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test VTK algo mesh filtering.

#include <test_gtest.h>
#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <vtkXMLPolyDataReader.h>

#include <arrows/vtk/algo/triangulate_mesh_filter.h>
#include <arrows/vtk/mesh_container.h>
#include <arrows/vtk/mesh_utils.h>

namespace kv = kwiver::vital;
namespace kav = kwiver::arrows::vtk;

kv::path_t g_data_dir;

static std::string vtp_file = "/mesh_files/triangles_and_quad.vtp";

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  GET_ARG( 1, g_data_dir );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( vtk_triangulate_mesh_filter, create )
{
  kv::plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    kv::create_algorithm< kv::algo::mesh_filter >( "vtk_triangulate" ) );
}

// ----------------------------------------------------------------------------
TEST ( vtk_triangulate_mesh_filter, filter )
{
  vtkSmartPointer< vtkXMLPolyDataReader > reader =
    vtkXMLPolyDataReader::New();
  reader->SetFileName( ( g_data_dir + vtp_file ).c_str() );
  reader->Update();

  kv::mesh_container_sptr cont =
    std::make_shared< kav::mesh_container >( reader->GetOutput() );

  auto triangulator = std::make_shared< kav::triangulate_mesh_filter >();
  auto vtk_out = triangulator->filter( cont );

  EXPECT_EQ( vtk_out->num_verts(), cont->num_verts() );
  EXPECT_EQ( cont->num_faces(), 5 );
  EXPECT_EQ( vtk_out->num_faces(), 6 );

  EXPECT_THROW( triangulator->filter( nullptr ), kv::invalid_value );
}
