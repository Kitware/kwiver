/*ckwg +29
 * Copyright 2016-2020 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief test class_map class
 */

#include <vital/types/class_map_types.h>
#include <gtest/gtest.h>

using namespace kwiver::vital;

namespace {

std::vector<std::string> const names =
  { "person", "vehicle", "other", "clam", "barnacle" };

std::vector<double> const scores  = { 0.65, 0.6, 0.07, 0.055, 0.005 };

}

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(activity_type, api)
{
  activity_type at( names, scores );

  EXPECT_EQ( 0.07, at.score( "other" ) );

  std::string ml_name;
  double ml_score;
  at.get_most_likely( ml_name, ml_score );

  EXPECT_EQ( "person", ml_name );
  EXPECT_EQ( 0.65, ml_score );

  for ( size_t i = 0; i < names.size(); ++i )
  {
    SCOPED_TRACE(
      "For score " + std::to_string( i ) + " ('" + names[i] + "')" );

    EXPECT_EQ( scores[i], at.score( names[i] ) );
  }

  EXPECT_EQ( 0.055, at.score( "clam" ) );

  at.set_score( "clam", 1.23 );
  EXPECT_EQ( 1.23, at.score( "clam" ) );

  EXPECT_EQ( 5, at.class_names().size() );

  EXPECT_NO_THROW( at.score( "other" ) ); // make sure this entry exists
  at.delete_score( "other" );
  EXPECT_THROW( at.score("other"), std::runtime_error )
    << "Accessing deleted class name";

  EXPECT_EQ( 4, at.class_names().size() );

  for ( auto const& name : at.class_names() )
  {
    std::cout << " -- " << name << "    score: "
              << at.score( name ) << std::endl;
  }

}

// ----------------------------------------------------------------------------
TEST(activity_type, creation_error)
{
  auto wrong_size_scores = scores;
  wrong_size_scores.resize( 4 );

  EXPECT_THROW(
    activity_type at( {}, {} ),
    std::invalid_argument );

  EXPECT_THROW(
    activity_type at( names, wrong_size_scores ),
    std::invalid_argument );
}

// ----------------------------------------------------------------------------
TEST(activity_type, name_pool)
{
  activity_type at( names, scores );

  std::vector<std::string> alt_names =
    { "a-person", "a-vehicle", "a-other", "a-clam", "a-barnacle" };

  activity_type at_2( alt_names, scores );

  EXPECT_EQ( 10, activity_type::all_class_names().size() );

  EXPECT_EQ( 0, detected_object_type::all_class_names().size() );
  
  for ( auto const& name : activity_type::all_class_names() )
  {
    std::cout << "  --  " << name << std::endl;
  }

}
