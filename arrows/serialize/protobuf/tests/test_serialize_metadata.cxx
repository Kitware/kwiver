// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test protobuf serializers

#include <gtest/gtest.h>

#include <arrows/serialize/protobuf/metadata.h>
#include <arrows/serialize/protobuf/convert_protobuf.h>

#include <vital/types/metadata.h>
#include <vital/types/metadata_tags.h>
#include <vital/types/metadata_traits.h>
#include <vital/types/polygon.h>
#include <vital/types/geo_polygon.h>
#include <vital/types/geodesy.h>

namespace kasp = kwiver::arrows::serialize::protobuf;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// 1) Test single metadata collection in vector
// 2) Test two different metadata collections in vector
// 3) Build metadata collection with one element of each supported type.

// ----------------------------------------------------------------------------
TEST( serialize_metadata, metadata )
{
  // Create a metadata collection
  // duplicate that collection and make some modifications
  // put both collections in a vector

  auto meta_sptr = std::make_shared< kwiver::vital::metadata>();
  meta_sptr->add< kwiver::vital::VITAL_META_METADATA_ORIGIN >( "test-source" );
  meta_sptr->add< kwiver::vital::VITAL_META_UNIX_TIMESTAMP >( 12345678 );
  meta_sptr->add< kwiver::vital::VITAL_META_SENSOR_VERTICAL_FOV >( 12345.678 );

  {
    kwiver::vital::geo_point::geo_2d_point_t geo_2d{ 42.50, 73.54 };
    kwiver::vital::geo_point pt{ geo_2d, kwiver::vital::SRID::lat_lon_WGS84 };
    meta_sptr->add< kwiver::vital::VITAL_META_FRAME_CENTER >( pt );
  }

  {
    kwiver::vital::geo_point::geo_3d_point_t geo{ 42.50, 73.54, 16.33 };
    kwiver::vital::geo_point pt{ geo, kwiver::vital::SRID::lat_lon_WGS84 };
    meta_sptr->add< kwiver::vital::VITAL_META_FRAME_CENTER >( pt );
  }

  {
    kwiver::vital::polygon raw_obj;
    raw_obj.push_back( 100, 100 );
    raw_obj.push_back( 400, 100 );
    raw_obj.push_back( 400, 400 );
    raw_obj.push_back( 100, 400 );
    kwiver::vital::geo_polygon poly( raw_obj,
                                     kwiver::vital::SRID::lat_lon_WGS84 );
    meta_sptr->add< kwiver::vital::VITAL_META_CORNER_POINTS >( poly );
  }

  kasp::metadata meta_ser;      // The serializer

  kwiver::vital::metadata_vector mvec;
  mvec.push_back( meta_sptr );
  mvec.push_back( meta_sptr ); // just so there is more than one

  std::any meta_any( mvec );
  auto mes = meta_ser.serialize( meta_any );
  auto meta_dser_any = meta_ser.deserialize( *mes );

  kwiver::vital::metadata_vector meta_dser =
    std::any_cast< kwiver::vital::metadata_vector >( meta_dser_any );

  // test for equality
  EXPECT_TRUE( test_equal_content( *meta_sptr, *meta_dser[0] ));
}
