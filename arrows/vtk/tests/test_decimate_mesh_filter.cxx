// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test VTK algo mesh filtering.

#include <test_gtest.h>
#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTriangleFilter.h>

#include <vital/config/config_block_io.h>

#include <arrows/vtk/algo/decimate_mesh_filter.h>
#include <arrows/vtk/mesh_container.h>
#include <arrows/vtk/mesh_utils.h>

namespace kv = kwiver::vital;
namespace kav = kwiver::arrows::vtk;

kv::path_t g_data_dir;

static std::string conf_file = "/config_files/vtk_decimate_mesh_filter.conf";

// ----------------------------------------------------------------------------
// Generate a triangulated sphere with ~5000 faces
static vtkSmartPointer< vtkPolyData >
make_sphere_mesh()
{
  vtkNew< vtkSphereSource > sphere;
  sphere->SetThetaResolution( 50 );
  sphere->SetPhiResolution( 50 );
  sphere->Update();

  // vtkSphereSource emits quads at the poles; triangulate to be safe.
  vtkNew< vtkTriangleFilter > tri;
  tri->SetInputConnection( sphere->GetOutputPort() );
  tri->Update();

  return tri->GetOutput();
}

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  GET_ARG( 1, g_data_dir );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( vtk_decimate_mesh_filter, create )
{
  kv::plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    kv::create_algorithm< kv::algo::mesh_filter >( "vtk_decimate" ) );
}

// ----------------------------------------------------------------------------
TEST ( vtk_decimate_mesh_filter, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    kav::decimate_mesh_filter,
    "Use VTK's DecimatePro algorithm to decimate mesh.",
    PARAM_DEFAULT(
      target_reduction, double,
      "Specify the desired reduction in the total number of polygons "
      "(e.g., if TargetReduction is set to 0.9, this filter will try to "
      "reduce the data set to 10% of its original size).",
      0.9 ),
    PARAM_DEFAULT(
      preserve_topology, bool,
      "Turn on/off whether to preserve the topology of the original mesh. "
      "If on, mesh splitting and hole elimination will not occur.",
      false ),
    PARAM_DEFAULT(
      feature_angle, double,
      "Specify the mesh feature angle. This angle is used to define what "
      "an edge is (i.e., if the surface normal between two adjacent "
      "triangles is >= FeatureAngle, an edge exists).",
      15.0 ),
    PARAM_DEFAULT(
      splitting, bool,
      "Turn on/off the splitting of the mesh at corners, along edges, at "
      "non-manifold points, or anywhere else a split is required.",
      true ),
    PARAM_DEFAULT(
      split_angle, double,
      "Specify the mesh split angle. This angle is used to control the "
      "splitting of the mesh.",
      75.0 ),
    PARAM_DEFAULT(
      pre_split_mesh, bool,
      "In some cases you may wish to split the mesh prior to algorithm "
      "execution. This separates the mesh into semi-planar patches.",
      false ),
    PARAM_DEFAULT(
      maximum_error, double,
      "Set the largest decimation error that is allowed during the "
      "decimation process.",
      VTK_DOUBLE_MAX ),
    PARAM_DEFAULT(
      accumulate_error, bool,
      "The computed error can either be computed directly from the mesh "
      "or the error may be accumulated as the mesh is modified.",
      false ),
    PARAM_DEFAULT(
      error_is_absolute, int,
      "The MaximumError is normally defined as a fraction of the dataset "
      "bounding diagonal. By setting ErrorIsAbsolute to 1, the error is "
      "instead defined as that specified by AbsoluteError.",
      0 ),
    PARAM_DEFAULT(
      absolute_error, double,
      "Same as MaximumError, but to be used when ErrorIsAbsolute is 1.",
      VTK_DOUBLE_MAX ),
    PARAM_DEFAULT(
      boundary_vertex_deletion, bool,
      "Turn on/off the deletion of vertices on the boundary of a mesh.",
      true ),
    PARAM_DEFAULT(
      degree, int,
      "If the number of triangles connected to a vertex exceeds \"Degree\", "
      "then the vertex will be split.",
      25 ),
    PARAM_DEFAULT(
      inflection_point_ratio, double,
      "Specify the inflection point ratio. An inflection point occurs when "
      "the ratio of reduction error between two iterations is >= "
      "InflectionPointRatio.",
      10.0 )
  );
}

// ----------------------------------------------------------------------------
TEST ( vtk_decimate_mesh_filter, filter )
{
  auto mesh_vtk = make_sphere_mesh();
  kv::mesh mesh_vital = kav::vtk_to_vital( mesh_vtk );
  auto simple_cont =
    std::make_shared< kv::simple_mesh_container >( mesh_vital );
  auto vtk_cont = std::make_shared< kav::mesh_container >( mesh_vtk );

  auto decimator = std::make_shared< kav::decimate_mesh_filter >();
  auto config = decimator->get_configuration();
  config->merge_config( kv::read_config_file( g_data_dir + conf_file ) );
  decimator->set_configuration( config );

  // Test that decimator works with vtk mesh container
  auto vtk_out = decimator->filter( vtk_cont );

  // Test that decimator works with vital simple mesh container
  auto simple_out = decimator->filter( simple_cont );

  EXPECT_EQ( vtk_out->num_faces(), simple_out->num_faces() );
  EXPECT_EQ( vtk_out->num_verts(), simple_out->num_verts() );
}

// ----------------------------------------------------------------------------
TEST ( vtk_decimate_mesh_filter, approximate_reduction )
{
  kv::mesh_container_sptr in_container =
    std::make_shared< kav::mesh_container >( make_sphere_mesh() );

  auto decimator = std::make_shared< kav::decimate_mesh_filter >();

  // Test default reduction ratio (0.9)
  auto out_container = decimator->filter( in_container );
  double reduction_ratio = out_container->num_faces() /
                           ( double ) in_container->num_faces();
  EXPECT_NEAR( 1 - 0.9, reduction_ratio, 0.01 );

  // Test reduction ratio of 0.5
  auto config = decimator->get_configuration();
  config->merge_config( kv::read_config_file( g_data_dir + conf_file ) );
  decimator->set_configuration( config );

  out_container = decimator->filter( in_container );
  reduction_ratio = out_container->num_faces() /
                    ( double ) in_container->num_faces();
  EXPECT_NEAR( 1 - 0.5, reduction_ratio, 0.01 );
}
