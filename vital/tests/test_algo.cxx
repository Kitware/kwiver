// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Basic algorithm configuration tests

#include <iostream>

#include <vital/config/config_block.h>
#include <vital/config/config_block_io.h>

#include <gtest/gtest.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/match_features.h>
#include <vital/algo/track_features.h>
#include <vital/exceptions/algorithm.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/vital_types.h>

int
main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

TEST ( algo, registered_names )
{
  kwiver::vital::plugin_manager& vpm =
    kwiver::vital::plugin_manager::instance();

  vpm.load_all_plugins();


  std::vector< std::string > mf_names = vpm._impl_names( "match_features" );
  std::cout << "registered \"match_features\" implementations\n";
  for( auto const& name : mf_names )
  {
    std::cout << "  " << name << std::endl;
  }
}

TEST ( algo, create_from_name )
{
  kwiver::vital::plugin_manager& vpm =
    kwiver::vital::plugin_manager::instance();
  vpm.load_all_plugins();


  // create invalid algorithm
  kwiver::vital::algorithm_sptr empty;
  EXPECT_THROW(
  {
    empty =
      kwiver::vital::create_algorithm< kwiver::vital::algo::match_features >(
        "not_a_real_impl" ); }, kwiver::vital::plugin_factory_not_found );

  // has algorithm type
  EXPECT_TRUE(
    !vpm.get_factories( "match_features" ).empty() );

  // does not have algorithm type
  EXPECT_TRUE(
    vpm.get_factories( "not_a_real_type" ).empty() );

  // has algorithm impl
  EXPECT_TRUE(
    kwiver::vital::has_algorithm_impl_name< kwiver::vital::algo::match_features >
    (
      "homography_guided" ) );
  // does not have algorithm impl
  EXPECT_FALSE(
    kwiver::vital::has_algorithm_impl_name< kwiver::vital::algo::match_features >
      ( "not_a_real_impl" ) );


  //  create valid algorithm
  auto valid = kwiver::vital::create_algorithm< kwiver::vital::algo::match_features >( "homography_guided" );
  EXPECT_NE( valid, nullptr );


  // create correct type
  kwiver::vital::algo::match_features_sptr mf = std::dynamic_pointer_cast< kwiver::vital::algo::match_features >( valid );
  EXPECT_EQ( !mf, false );
  // create correct impl
  EXPECT_EQ( mf->impl_name(), "homography_guided" );
}

TEST ( algo,  track_features_before_configuration )
{
  // register algorithms from plugins
  kwiver::vital::plugin_manager& vpm =
    kwiver::vital::plugin_manager::instance();
  vpm.load_all_plugins();


  kwiver::vital::algo::track_features_sptr track_features_impl =
    kwiver::vital::create_algorithm< kwiver::vital::algo::track_features >(
      "core" );

  std::cerr <<
    "Contents of kwiver::vital::config_block BEFORE attempted configuration:" <<
    std::endl;


  kwiver::vital::config_block_sptr tf_config =
    track_features_impl->get_configuration();
  kwiver::vital::write_config( tf_config, std::cerr );

  std::cerr << "Setting mf algo impl" << std::endl;
  tf_config->set_value( "feature_matcher:type", "homography_guided" );

  std::cerr << "Contents of kwiver::vital::config_block after cb set:" <<
    std::endl;
  kwiver::vital::write_config( tf_config, std::cerr );

  std::cerr << "Setting modified config to tf algorithm" << std::endl;
  track_features_impl->set_configuration( tf_config );

  std::cerr << "algo's config after set:" << std::endl;
  tf_config = track_features_impl->get_configuration();
  kwiver::vital::write_config( tf_config, std::cerr );

  std::cerr << "Setting mf's mf algo type (in config)" << std::endl;
  tf_config->set_value(
    "feature_matcher:homography_guided:feature_matcher:type",
    "homography_guided" );

  std::cerr << "Contents of kwiver::vital::config_block after set:" <<
    std::endl;
  kwiver::vital::write_config( tf_config, std::cerr );

  track_features_impl->set_configuration( tf_config );

  std::cerr << "algo's config after second algo set:" << std::endl;
  tf_config = track_features_impl->get_configuration();
  kwiver::vital::write_config( tf_config, std::cerr );

  std::cerr << "One more level for good measure" << std::endl;
  tf_config->set_value(
    "feature_matcher:homography_guided:feature_matcher:homography_guided:feature_matcher:type",
    "homography_guided" );

  std::cerr << "Contents of cb after set:" << std::endl;
  kwiver::vital::write_config( tf_config, std::cerr );

  track_features_impl->set_configuration( tf_config );

  std::cerr << "algo's config after third algo set" << std::endl;
  tf_config = track_features_impl->get_configuration();
  kwiver::vital::write_config( tf_config, std::cerr );

  std::cerr << "One more level for good measure" << std::endl;
  tf_config->set_value(
    "feature_matcher:homography_guided:feature_matcher:homography_guided:feature_matcher:homography_guided:feature_matcher:type",
    "homography_guided" );

  std::cerr << "Contents of cb after set:" << std::endl;
  kwiver::vital::write_config( tf_config, std::cerr );

  track_features_impl->set_configuration( tf_config );

  std::cerr << "algo's config after third algo set" << std::endl;
  tf_config = track_features_impl->get_configuration();
  kwiver::vital::write_config( tf_config, std::cerr );
}
TEST ( algo,  track_features_check_config )
{
  // register algorithms from plugins
  kwiver::vital::plugin_manager& vpm =
    kwiver::vital::plugin_manager::instance();
  vpm.load_all_plugins();


  kwiver::vital::algo::track_features_sptr tf_impl =
    kwiver::vital::create_algorithm< kwiver::vital::algo::track_features >(
      "core" );

// Checking that exception is thrown when trying to configure with no config
// parameters.
  kwiver::vital::config_block_sptr config =
    kwiver::vital::config_block::empty_config( "track_features_check_config" );
  EXPECT_FALSE( tf_impl->check_configuration( config ) );

  // Checking that default impl switch value is invalid (base default is
  // nothing).
  config = tf_impl->get_configuration();
  std::cerr << "Default config:" << std::endl;
  kwiver::vital::write_config( config, std::cerr );
  // default config check
  EXPECT_FALSE( tf_impl->check_configuration( config ) );
  // Adding valid implementation name for match_features algo, but should
  // still fail as the underlying match_features impl wants another nested
  // algo specification.
  config->set_value( "feature_matcher:type", "homography_guided" );
  std::cerr << "Modified configuration:" << std::endl;
  kwiver::vital::write_config( config, std::cerr );
  EXPECT_FALSE( tf_impl->check_configuration( config ) );

  std::cerr << "Config from perspective of algo with that that config:" <<
    std::endl;
  tf_impl->set_configuration( config );


  kwiver::vital::config_block_sptr cb = tf_impl->get_configuration();
  kwiver::vital::write_config( config, std::cerr );

  // Checking that, even though there were nested algorithms that weren't set,
  // at least the one that we did set propaged correctly and triggered the
  // sub-config generation.
  // param check 1
  EXPECT_EQ(
    cb->get_value< std::string >( "feature_matcher:type" ),
    "homography_guided" );
  // param check 2
  EXPECT_TRUE(
    cb->has_value(
      "feature_matcher:homography_guided:feature_matcher1:type" ) );
}
