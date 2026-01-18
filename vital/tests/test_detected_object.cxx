// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test detected_object class

#include <vital/attribute_set.h>
#include <vital/types/detected_object.h>
#include <vital/types/geodesy.h>

#include <gtest/gtest.h>

static auto const loc = kwiver::vital::vector_3d{ -73.759291, 42.849631, 50 };
static auto constexpr crs = kwiver::vital::SRID::lat_lon_WGS84;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( detected_object, creation )
{
  kwiver::vital::bounding_box_d::vector_type tl{ 12, 23 };
  kwiver::vital::bounding_box_d bb{ tl, 100, 100 };

  kwiver::vital::detected_object dobj{ bb };

  // Test expected values passed to constructor
  EXPECT_EQ( 1.0, dobj.confidence() );
  EXPECT_EQ( bb.upper_left(), dobj.bounding_box().upper_left() );
  EXPECT_EQ( bb.lower_right(), dobj.bounding_box().lower_right() );
  EXPECT_EQ( nullptr, dobj.type() );

  // Test expected default values
  EXPECT_EQ( 0, dobj.index() );
  EXPECT_EQ( "", dobj.detector_name() );

  kwiver::vital::geo_point gp{ loc, crs };
  kwiver::vital::detected_object dobj2{ gp };

  // Test expected values passed to constructor
  EXPECT_EQ( loc, dobj2.geo_point().location( crs ) );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, modification )
{
  kwiver::vital::bounding_box_d::vector_type tl{ 12, 23 };
  kwiver::vital::bounding_box_d bb{ tl, 100, 100 };

  kwiver::vital::detected_object dobj{ bb };

  dobj.set_index( 1234 );
  EXPECT_EQ( 1234, dobj.index() );

  dobj.set_detector_name( "foo detector" );
  EXPECT_EQ( "foo detector", dobj.detector_name() );

  dobj.set_confidence( 0.5546 );
  EXPECT_EQ( 0.5546, dobj.confidence() );

  auto const names = std::vector< std::string >{
    "person",
    "vehicle",
    "other",
    "clam",
    "barnacle", };

  auto const scores = std::vector< double >{
    0.65,
    0.6,
    0.07,
    0.0055,
    0.005, };

  auto const dot =
    std::make_shared< kwiver::vital::detected_object_type >( names, scores );
  dobj.set_type( dot );
  EXPECT_EQ( dot, dobj.type() );

  kwiver::vital::bounding_box_d::vector_type offset{ 20, 10 };
  kwiver::vital::translate( bb, offset );
  dobj.set_bounding_box( bb );
  EXPECT_EQ( bb.upper_left(), dobj.bounding_box().upper_left() );
  EXPECT_EQ( bb.lower_right(), dobj.bounding_box().lower_right() );

  dobj.set_geo_point( { loc, crs } );
  EXPECT_EQ( loc, dobj.geo_point().location( crs ) );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, keypoints )
{
  kwiver::vital::detected_object dobj;

  dobj.add_keypoint( "head", { 11.2, 13.1 } );
  dobj.add_keypoint( "head", { 4.2, 9.5 } );

  auto const& keypoints = dobj.keypoints();
  EXPECT_EQ( 1, keypoints.size() );
  ASSERT_EQ( 1, keypoints.count( "head" ) );

  EXPECT_EQ( 4.2, keypoints.find( "head" )->second.value()[ 0 ] );
  EXPECT_EQ( 9.5, keypoints.find( "head" )->second.value()[ 1 ] );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, notes )
{
  kwiver::vital::detected_object dobj;

  EXPECT_EQ( dobj.notes().size(), 0 );

  dobj.add_note( "Dogs have owners." );
  dobj.add_note( "Cats have staff." );

  auto const& notes = dobj.notes();
  ASSERT_EQ( notes.size(), 2 );
  EXPECT_EQ( "Dogs have owners.", notes[ 0 ] );
  EXPECT_EQ( "Cats have staff.", notes[ 1 ] );

  dobj.clear_notes();
  EXPECT_EQ( dobj.notes().size(), 0 );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, attributes )
{
  kwiver::vital::detected_object dobj;

  // Initially no attributes
  EXPECT_EQ( nullptr, dobj.attributes() );

  // Create and attach attribute set
  auto attrs = std::make_shared< kwiver::vital::attribute_set >();
  dobj.set_attributes( attrs );
  EXPECT_NE( nullptr, dobj.attributes() );
  EXPECT_TRUE( dobj.attributes()->empty() );

  // Add various typed attributes
  attrs->add( "string_attr", std::string( "test_value" ) );
  attrs->add( "int_attr", 42 );
  attrs->add( "double_attr", 3.14159 );
  attrs->add( "bool_attr", true );

  EXPECT_EQ( 4, dobj.attributes()->size() );

  // Verify attribute values and types
  EXPECT_TRUE( dobj.attributes()->has( "string_attr" ) );
  EXPECT_TRUE( dobj.attributes()->has( "int_attr" ) );
  EXPECT_TRUE( dobj.attributes()->has( "double_attr" ) );
  EXPECT_TRUE( dobj.attributes()->has( "bool_attr" ) );
  EXPECT_FALSE( dobj.attributes()->has( "nonexistent" ) );

  EXPECT_EQ(
    "test_value",
    dobj.attributes()->get< std::string >( "string_attr" ) );
  EXPECT_EQ( 42, dobj.attributes()->get< int >( "int_attr" ) );
  EXPECT_DOUBLE_EQ(
    3.14159,
    dobj.attributes()->get< double >( "double_attr" ) );
  EXPECT_TRUE( dobj.attributes()->get< bool >( "bool_attr" ) );

  // Test type checking
  EXPECT_TRUE( dobj.attributes()->is_type< std::string >( "string_attr" ) );
  EXPECT_TRUE( dobj.attributes()->is_type< int >( "int_attr" ) );
  EXPECT_FALSE( dobj.attributes()->is_type< std::string >( "int_attr" ) );

  // Test erase
  EXPECT_TRUE( dobj.attributes()->erase( "int_attr" ) );
  EXPECT_FALSE( dobj.attributes()->has( "int_attr" ) );
  EXPECT_EQ( 3, dobj.attributes()->size() );
  EXPECT_FALSE( dobj.attributes()->erase( "nonexistent" ) );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, attributes_clone )
{
  kwiver::vital::bounding_box_d bb{ { 10, 20 }, 100, 100 };
  kwiver::vital::detected_object dobj{ bb, 0.9 };

  // Create and attach attribute set
  auto attrs = std::make_shared< kwiver::vital::attribute_set >();
  attrs->add( "label", std::string( "person" ) );
  attrs->add( "score", 0.95 );
  dobj.set_attributes( attrs );

  // Clone the detection
  auto cloned = dobj.clone();

  // Verify attributes are deep copied
  EXPECT_NE( nullptr, cloned->attributes() );
  EXPECT_NE( dobj.attributes().get(), cloned->attributes().get() );
  EXPECT_EQ( 2, cloned->attributes()->size() );
  EXPECT_EQ( "person", cloned->attributes()->get< std::string >( "label" ) );
  EXPECT_DOUBLE_EQ( 0.95, cloned->attributes()->get< double >( "score" ) );

  // Modify original, verify clone is independent
  dobj.attributes()->add( "new_attr", 123 );
  EXPECT_TRUE( dobj.attributes()->has( "new_attr" ) );
  EXPECT_FALSE( cloned->attributes()->has( "new_attr" ) );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, attributes_null_clone )
{
  kwiver::vital::detected_object dobj;

  // No attributes set
  EXPECT_EQ( nullptr, dobj.attributes() );

  // Clone should also have no attributes
  auto cloned = dobj.clone();
  EXPECT_EQ( nullptr, cloned->attributes() );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, set_attribute_convenience )
{
  kwiver::vital::detected_object dobj;

  // Initially no attributes
  EXPECT_EQ( nullptr, dobj.attributes() );
  EXPECT_FALSE( dobj.has_attribute( "test" ) );

  // set_attribute should create attribute set automatically
  dobj.set_attribute( "string_val", std::string( "hello" ) );
  EXPECT_NE( nullptr, dobj.attributes() );
  EXPECT_TRUE( dobj.has_attribute( "string_val" ) );
  EXPECT_EQ( "hello", dobj.get_attribute< std::string >( "string_val" ) );

  // Add more attributes of different types
  dobj.set_attribute( "int_val", 42 );
  dobj.set_attribute( "double_val", 3.14159 );
  dobj.set_attribute( "bool_val", true );

  EXPECT_EQ( 4, dobj.attributes()->size() );
  EXPECT_TRUE( dobj.has_attribute( "int_val" ) );
  EXPECT_TRUE( dobj.has_attribute( "double_val" ) );
  EXPECT_TRUE( dobj.has_attribute( "bool_val" ) );

  EXPECT_EQ( 42, dobj.get_attribute< int >( "int_val" ) );
  EXPECT_DOUBLE_EQ( 3.14159, dobj.get_attribute< double >( "double_val" ) );
  EXPECT_TRUE( dobj.get_attribute< bool >( "bool_val" ) );

  // Overwrite existing attribute
  dobj.set_attribute( "int_val", 100 );
  EXPECT_EQ( 100, dobj.get_attribute< int >( "int_val" ) );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, get_attribute_exception )
{
  kwiver::vital::detected_object dobj;

  // get_attribute on detection with no attributes should throw
  EXPECT_THROW(
    dobj.get_attribute< int >( "nonexistent" ),
    kwiver::vital::attribute_set_exception );

  // Create attribute set but don't add the requested attribute
  dobj.set_attribute( "other", 123 );
  EXPECT_THROW(
    dobj.get_attribute< int >( "nonexistent" ),
    kwiver::vital::attribute_set_exception );
}

// ----------------------------------------------------------------------------
TEST ( detected_object, set_attribute_with_existing_attrs )
{
  kwiver::vital::detected_object dobj;

  // First set attributes via set_attributes
  auto attrs = std::make_shared< kwiver::vital::attribute_set >();
  attrs->add( "existing", std::string( "value" ) );
  dobj.set_attributes( attrs );

  EXPECT_EQ( 1, dobj.attributes()->size() );

  // Now use set_attribute to add more
  dobj.set_attribute( "new_attr", 42 );

  // Both should exist
  EXPECT_EQ( 2, dobj.attributes()->size() );
  EXPECT_TRUE( dobj.has_attribute( "existing" ) );
  EXPECT_TRUE( dobj.has_attribute( "new_attr" ) );
  EXPECT_EQ( "value", dobj.get_attribute< std::string >( "existing" ) );
  EXPECT_EQ( 42, dobj.get_attribute< int >( "new_attr" ) );
}
