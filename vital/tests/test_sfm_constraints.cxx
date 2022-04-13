// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test sfm_constraints class

#include <arrows/proj/geo_conv.h>
#include <vital/types/sfm_constraints.h>
#include <vital/types/metadata.h>
#include <vital/types/metadata_map.h>
#include <vital/types/metadata_tags.h>
#include <vital/types/geodesy.h>
#include <gtest/gtest.h>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

template < vital_metadata_tag Tag >
void
add_metadata(metadata_map::map_metadata_t &mdm,
             frame_id_t frame_id,
             type_of_tag<Tag> value)
{
  metadata_sptr md = std::make_shared<metadata>();
  md->add< Tag >( value );

  if (mdm.find(frame_id) != mdm.end())
  {
    mdm[frame_id].push_back( md );
  }
  else
  {
    mdm[frame_id] = metadata_vector{ md };
  }
}

// ----------------------------------------------------------------------------
TEST(sfm_constraints, create)
{
  EXPECT_NE( nullptr, std::make_shared<sfm_constraints>() );
}

// ----------------------------------------------------------------------------
TEST(sfm_constraints, constructors)
{
  // Create dummy metadata map
  metadata_map::map_metadata_t mdm;
  for (frame_id_t i = 0; i < 10; ++i)
  {
    add_metadata<VITAL_META_UNIX_TIMESTAMP>(mdm, i, i);
  }
  metadata_map_sptr metadata_map = std::make_shared<simple_metadata_map>(mdm);
  EXPECT_EQ(metadata_map->size(), 10);

  sfm_constraints constraints;
  EXPECT_EQ(constraints.get_metadata(), nullptr);
  EXPECT_EQ(constraints.get_local_geo_cs().origin().crs(), -1);

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  set_geo_conv(&geo_conv);
  local_geo_cs lgcs;
  lgcs.set_origin(geo_point(vector_3d(0, 0, 0), SRID::lat_lon_WGS84));

  constraints.set_metadata(metadata_map);
  constraints.set_local_geo_cs(lgcs);

  auto constraints_1 = sfm_constraints(constraints);
  EXPECT_EQ(constraints_1.get_metadata()->size(), 10);
  EXPECT_EQ(constraints.get_metadata(), constraints_1.get_metadata());

  EXPECT_EQ(constraints.get_local_geo_cs().origin().crs(),
            constraints_1.get_local_geo_cs().origin().crs());

  sfm_constraints constraints_2(metadata_map, lgcs);
  EXPECT_EQ(constraints_2.get_metadata()->size(), 10);
  EXPECT_EQ(constraints.get_metadata(), constraints_2.get_metadata());
  EXPECT_EQ(constraints.get_local_geo_cs().origin().crs(),
            constraints_2.get_local_geo_cs().origin().crs());
}

// ----------------------------------------------------------------------------
TEST(sfm_constraints, get_focal_length_prior)
{
  sfm_constraints constraints;
  float focal_length = 1.0f;

  // Expect failure if the metadata map is not set
  EXPECT_FALSE(constraints.get_focal_length_prior(0, focal_length));
  EXPECT_EQ(focal_length, 1.0f);

  // Create metadata map
  metadata_map::map_metadata_t mdm;
  add_metadata<VITAL_META_SENSOR_HORIZONTAL_FOV>(mdm, 0, 90.0);
  add_metadata<VITAL_META_SLANT_RANGE>(mdm, 1, 10.0);
  add_metadata<VITAL_META_TARGET_WIDTH>(mdm, 1, 10.0);
  metadata_map_sptr metadata_map = std::make_shared<simple_metadata_map>(mdm);
  constraints.set_metadata(metadata_map);
  EXPECT_EQ(constraints.get_metadata()->size(), 2);

  // Expect failure if the size is not set
  EXPECT_FALSE(constraints.get_focal_length_prior(0, focal_length));
  EXPECT_EQ(focal_length, 1.0f);

  constraints.store_image_size(0, 10, 10);
  constraints.store_image_size(1, 10, 10);
  constraints.store_image_size(2, 10, 10);

  // Success cases
  EXPECT_TRUE(constraints.get_focal_length_prior(0, focal_length));
  EXPECT_EQ(focal_length, 5.0f);

  focal_length = 1.0f;
  EXPECT_TRUE(constraints.get_focal_length_prior(1, focal_length));
  EXPECT_EQ(focal_length, 10.0f);

  focal_length = 1.0f;
  EXPECT_TRUE(constraints.get_focal_length_prior(-1, focal_length));
  EXPECT_EQ(focal_length, 10.0f);

  // Expect failure if the frame_id has no metadata
  focal_length = 1.0f;
  EXPECT_FALSE(constraints.get_focal_length_prior(2, focal_length));
  EXPECT_EQ(focal_length, 1.0f);
}

// ----------------------------------------------------------------------------
TEST(sfm_constraints, get_camera_orientation_prior_local)
{
  sfm_constraints constraints;
  rotation_d R_loc;

  // Expect failure if the lgcs is not set
  EXPECT_FALSE(constraints.get_camera_orientation_prior_local(0, R_loc));
  EXPECT_EQ(R_loc, rotation_d{});

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  set_geo_conv(&geo_conv);
  local_geo_cs lgcs;
  lgcs.set_origin(geo_point(vector_3d(0, 0, 0), SRID::lat_lon_WGS84));
  constraints.set_local_geo_cs(lgcs);

  // Expect failure if the metadata map is not set
  EXPECT_FALSE(constraints.get_camera_orientation_prior_local(0, R_loc));
  EXPECT_EQ(R_loc, rotation_d{});

  // Create metadata map
  metadata_map::map_metadata_t mdm;
  add_metadata<VITAL_META_PLATFORM_HEADING_ANGLE>(mdm, 0, 90.0);
  add_metadata<VITAL_META_PLATFORM_ROLL_ANGLE>(mdm, 0, 90.0);
  add_metadata<VITAL_META_PLATFORM_PITCH_ANGLE>(mdm, 0, 90.0);
  add_metadata<VITAL_META_SENSOR_REL_AZ_ANGLE>(mdm, 0, 90.0);
  add_metadata<VITAL_META_SENSOR_REL_EL_ANGLE>(mdm, 0, 90.0);
  add_metadata<VITAL_META_SENSOR_REL_ROLL_ANGLE>(mdm, 0, 90.0);
  add_metadata<VITAL_META_SENSOR_REL_ROLL_ANGLE>(mdm, 1, 90.0);
  add_metadata<VITAL_META_PLATFORM_HEADING_ANGLE>(mdm, 2, 90.0);
  add_metadata<VITAL_META_PLATFORM_ROLL_ANGLE>(mdm, 2, 90.0);
  add_metadata<VITAL_META_PLATFORM_PITCH_ANGLE>(mdm, 2, 90.0);
  add_metadata<VITAL_META_SENSOR_REL_AZ_ANGLE>(mdm, 2, 90.0);
  add_metadata<VITAL_META_SENSOR_REL_EL_ANGLE>(mdm, 2, 90.0);
  add_metadata<VITAL_META_SENSOR_REL_ROLL_ANGLE>(mdm, 2, std::nan(""));
  metadata_map_sptr metadata_map = std::make_shared<simple_metadata_map>(mdm);
  constraints.set_metadata(metadata_map);
  EXPECT_EQ(constraints.get_metadata()->size(), 3);

  // Expect failure if the metadata values are not set
  EXPECT_FALSE(constraints.get_camera_orientation_prior_local(1, R_loc));
  EXPECT_EQ(R_loc, rotation_d{});

  // Expect failure if a metadata value is nan
  EXPECT_FALSE(constraints.get_camera_orientation_prior_local(2, R_loc));
  EXPECT_EQ(R_loc, rotation_d{});

  // Success case
  EXPECT_TRUE(constraints.get_camera_orientation_prior_local(0, R_loc));
  EXPECT_NEAR(R_loc.quaternion().coeffs()[0], 0.0446255, 1e-6);
  EXPECT_NEAR(R_loc.quaternion().coeffs()[1], -0.26865, 1e-6);
  EXPECT_NEAR(R_loc.quaternion().coeffs()[2], -0.818742, 1e-6);
  EXPECT_NEAR(R_loc.quaternion().coeffs()[3], 0.505467, 1e-6);
}

// ----------------------------------------------------------------------------
TEST(sfm_constraints, get_camera_position_prior_local)
{
  sfm_constraints constraints;
  vector_3d pos_loc(1, 1, 1);

  // Expect failure if the lgcs is not set
  EXPECT_FALSE(constraints.get_camera_position_prior_local(0, pos_loc));
  EXPECT_EQ(pos_loc.x(), 1.0);
  EXPECT_EQ(pos_loc.y(), 1.0);
  EXPECT_EQ(pos_loc.z(), 1.0);

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  set_geo_conv(&geo_conv);
  local_geo_cs lgcs;
  lgcs.set_origin(geo_point(vector_3d(1, 1, 1), SRID::lat_lon_WGS84));
  constraints.set_local_geo_cs(lgcs);

  // Expect failure if the metadata map is not set
  EXPECT_FALSE(constraints.get_camera_position_prior_local(0, pos_loc));
  EXPECT_EQ(pos_loc.x(), 1.0);
  EXPECT_EQ(pos_loc.y(), 1.0);
  EXPECT_EQ(pos_loc.z(), 1.0);

  // Create metadata map
  metadata_map::map_metadata_t mdm;
  add_metadata<VITAL_META_SENSOR_LOCATION>(mdm, 0,
    geo_point(vector_3d(1, 1, 3), SRID::lat_lon_WGS84));
  metadata_map_sptr metadata_map = std::make_shared<simple_metadata_map>(mdm);
  constraints.set_metadata(metadata_map);
  EXPECT_EQ(constraints.get_metadata()->size(), 1);

  // Expect failure if the metadata values are not set
  EXPECT_FALSE(constraints.get_camera_position_prior_local(1, pos_loc));
  EXPECT_EQ(pos_loc.x(), 1.0);
  EXPECT_EQ(pos_loc.y(), 1.0);
  EXPECT_EQ(pos_loc.z(), 1.0);

  // Success case
  EXPECT_TRUE(constraints.get_camera_position_prior_local(0, pos_loc));
  EXPECT_EQ(pos_loc.x(), 0.0);
  EXPECT_EQ(pos_loc.y(), 0.0);
  EXPECT_EQ(pos_loc.z(), 2.0);
}

// ----------------------------------------------------------------------------
TEST(sfm_constraints, get_camera_position_priors)
{
  sfm_constraints constraints;

  // Expect size 0 position_map if the metadata is not set
  EXPECT_EQ(constraints.get_camera_position_priors().size(), 0);

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  set_geo_conv(&geo_conv);
  local_geo_cs lgcs;
  lgcs.set_origin(geo_point(vector_3d(1, 1, 1), SRID::lat_lon_WGS84));
  constraints.set_local_geo_cs(lgcs);

  // Create metadata map
  metadata_map::map_metadata_t mdm;
  add_metadata<VITAL_META_SENSOR_LOCATION>(mdm, 0,
    geo_point(vector_3d(1, 1, 2), SRID::lat_lon_WGS84));
  add_metadata<VITAL_META_SENSOR_LOCATION>(mdm, 1,
    geo_point(vector_3d(1, 1, 3), SRID::lat_lon_WGS84));
  add_metadata<VITAL_META_SENSOR_LOCATION>(mdm, 2,
    geo_point(vector_3d(1, 1, 3), SRID::lat_lon_WGS84));
  add_metadata<VITAL_META_UNKNOWN>(mdm, 4, 0);
  metadata_map_sptr metadata_map = std::make_shared<simple_metadata_map>(mdm);
  constraints.set_metadata(metadata_map);
  EXPECT_EQ(constraints.get_metadata()->size(), 4);

  auto pos_map = constraints.get_camera_position_priors();
  EXPECT_EQ(pos_map.size(), 2);
  EXPECT_EQ(pos_map[0].x(), 0.0);
  EXPECT_EQ(pos_map[0].y(), 0.0);
  EXPECT_EQ(pos_map[0].z(), 1.0);

  EXPECT_EQ(pos_map[1].x(), 0.0);
  EXPECT_EQ(pos_map[1].y(), 0.0);
  EXPECT_EQ(pos_map[1].z(), 2.0);
}

// ----------------------------------------------------------------------------
TEST(sfm_constraints, image_dimensions)
{
  sfm_constraints constraints;
  int dim = 0;

  EXPECT_FALSE(constraints.get_image_height(-1, dim));
  EXPECT_EQ(dim, 0);
  EXPECT_FALSE(constraints.get_image_width(-1, dim));
  EXPECT_EQ(dim, 0);

  constraints.store_image_size(0, 20, 10);

  EXPECT_TRUE(constraints.get_image_height(0, dim));
  EXPECT_EQ(dim, 10);
  EXPECT_TRUE(constraints.get_image_width(0, dim));
  EXPECT_EQ(dim, 20);

  EXPECT_TRUE(constraints.get_image_height(-1, dim));
  EXPECT_EQ(dim, 10);
  EXPECT_TRUE(constraints.get_image_width(-1, dim));
  EXPECT_EQ(dim, 20);

  dim = 0;
  EXPECT_FALSE(constraints.get_image_height(1, dim));
  EXPECT_EQ(dim, 0);
  EXPECT_FALSE(constraints.get_image_width(1, dim));
  EXPECT_EQ(dim, 0);
}
