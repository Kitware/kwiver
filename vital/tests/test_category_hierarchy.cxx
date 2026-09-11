// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/types/category_hierarchy.h>
#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>

using kwiver::vital::category_hierarchy;

class label_files : public ::testing::Test
{
protected:
  std::vector< std::string > paths;
  std::shared_ptr< category_hierarchy > load(
    const std::string& text, const std::string& extension = ".txt" )
  {
    const std::string path = ::testing::TempDir() + "kwiver_labels_" +
      ::testing::UnitTest::GetInstance()->current_test_info()->name() + extension;
    paths.push_back( path );
    {
      std::ofstream out( path, std::ios::binary );
      out << text;
      if( !out.good() ) { throw std::runtime_error( "Cannot write fixture" ); }
    }
    return std::make_shared< category_hierarchy >( path );
  }
  void TearDown() override
  {
    for( const auto& path : paths ) { std::remove( path.c_str() ); }
  }
};

TEST_F( label_files, quoted_txt_synonyms_and_parents )
{
  auto labels = load(
    "\xef\xbb\xbf# comment\r\n"
    "\"sport glove\" 'athletic glove' :parent=\"sport equipment\" # comment\r\n"
    "glove\r\n\"sport equipment\" gear\r\n" );
  EXPECT_EQ( labels->all_class_names(),
    ( std::vector< std::string >{ "sport glove", "glove", "sport equipment" } ) );
  EXPECT_EQ( labels->get_class_name( "athletic glove" ), "sport glove" );
  EXPECT_EQ( labels->get_class_synonyms( "sport glove" ),
    ( std::vector< std::string >{ "athletic glove" } ) );
  EXPECT_EQ( labels->get_class_parents( "athletic glove" ),
    ( std::vector< std::string >{ "sport equipment" } ) );
  EXPECT_EQ( labels->get_class_id( "athletic glove" ), 0 );
}

TEST_F( label_files, legacy_txt_synonyms_are_preserved )
{
  auto labels = load( "fish cod haddock\ncrab\n" );
  EXPECT_EQ( labels->get_class_name( "cod" ), "fish" );
  EXPECT_EQ( labels->get_class_name( "haddock" ), "fish" );
  EXPECT_EQ( labels->all_class_names().size(), 2 );
  EXPECT_THROW( load( "sport glove\nglove\n" ), std::runtime_error );
}

TEST_F( label_files, quoted_punctuation_is_literal )
{
  auto labels = load( "\"fish #1\" \"fish \\\"one\\\"\"\nmen's-glove\n" );
  EXPECT_EQ( labels->get_class_name( "fish \"one\"" ), "fish #1" );
  EXPECT_TRUE( labels->has_class_name( "men's-glove" ) );
}

TEST_F( label_files, csv_fields_preserve_spaces_commas_and_quotes )
{
  auto labels = load(
    "sport glove, athletic glove ,:parent=sport equipment\r\n"
    "glove\r\n sport equipment ,gear\r\n"
    "\"fish, red\",\"fish \"\"red\"\"\"\r\n", ".csv" );
  EXPECT_EQ( labels->get_class_name( "athletic glove" ), "sport glove" );
  EXPECT_EQ( labels->get_class_name( "fish \"red\"" ), "fish, red" );
  EXPECT_EQ( labels->get_class_parents( "sport glove" ),
    ( std::vector< std::string >{ "sport equipment" } ) );
}

TEST_F( label_files, json_string_array )
{
  auto labels = load( "[\"sport glove\",\"glove\"]", ".JSON" );
  EXPECT_EQ( labels->all_class_names(),
    ( std::vector< std::string >{ "sport glove", "glove" } ) );
}

TEST_F( label_files, json_dive_coco_hierarchy_and_synonyms )
{
  auto labels = load( R"({"categories":[
    {"id":2,"name":"sport glove","synonyms":["athletic glove"],
     "supercategory":"sport equipment","parents":["ignored fallback"]},
    {"id":1,"name":"glove"},
    {"id":3,"name":"sport equipment"}
  ]})", ".json" );
  EXPECT_EQ( labels->get_class_id( "athletic glove" ), 2 );
  EXPECT_EQ( labels->get_class_name( "athletic glove" ), "sport glove" );
  EXPECT_EQ( labels->get_class_parents( "sport glove" ),
    ( std::vector< std::string >{ "sport equipment" } ) );
  EXPECT_FALSE( labels->has_class_name( "ignored fallback" ) );
}

TEST_F( label_files, json_dive_type_hierarchy_and_implicit_parent )
{
  auto labels = load( R"({"categories":[
    {"name":"sport glove","synonyms":["athletic glove"]}, "glove"
  ], "typeHierarchy":{"sport glove":"sport equipment"}})", ".json" );
  EXPECT_EQ( labels->get_class_parents( "athletic glove" ),
    ( std::vector< std::string >{ "sport equipment" } ) );
  EXPECT_TRUE( labels->has_class_name( "sport equipment" ) );
}

TEST_F( label_files, json_multiple_parents )
{
  auto labels = load( R"([{"name":"sport glove",
    "parents":["sport equipment","glove"],"synonyms":["athletic glove"]}])", ".json" );
  EXPECT_EQ( labels->get_class_parents( "athletic glove" ),
    ( std::vector< std::string >{ "sport equipment", "glove" } ) );
}

TEST_F( label_files, json_dive_hierarchy_without_category_records )
{
  auto labels = load( R"({"typeHierarchy":{"sport glove":"sport equipment"}})", ".json" );
  EXPECT_EQ( labels->get_class_parents( "sport glove" ),
    ( std::vector< std::string >{ "sport equipment" } ) );
}

TEST_F( label_files, invalid_text_is_rejected )
{
  for( const auto& text : { "\"sport glove", "\"\"", "\"sport\"glove", "a :parent=missing" } )
  {
    EXPECT_THROW( load( text ), std::runtime_error ) << text;
  }
  EXPECT_THROW( load( ",alias", ".csv" ), std::runtime_error );
  EXPECT_THROW( load( "\"unterminated", ".csv" ), std::runtime_error );
}

TEST_F( label_files, invalid_json_is_rejected )
{
  for( const auto& text : { "{", "{}", "[42]", "[\"\"]",
    "[{\"name\":\"fish\",\"synonyms\":\"cod\"}]",
    "[{\"name\":\"fish\",\"id\":\"one\"}]",
    "[{\"name\":\"fish\",\"synonyms\":[\"cod\"]},\"cod\"]" } )
  {
    EXPECT_THROW( load( text, ".json" ), std::runtime_error ) << text;
  }
}

TEST_F( label_files, cycles_are_rejected )
{
  EXPECT_THROW( load( "a :parent=b\nb :parent=a" ), std::runtime_error );
  EXPECT_THROW( load( R"([{"name":"a","supercategory":"a"}])", ".json" ),
    std::runtime_error );
}

int main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
