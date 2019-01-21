/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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

#include <gtest/gtest.h>

#include <vital/types/image.h>
#include <vital/types/image_container_set_simple.h>


namespace {

auto test_logger = kwiver::vital::get_logger( "vital.tests.test_image_container_set" );

}


// ----------------------------------------------------------------------------
int
main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
// Helper functions / classes for tests
namespace {

using ic_vec_t = std::vector< kwiver::vital::image_container_sptr >;

/**
 * Create simple vector of image_container_sptr instances.
 *
 * Output will consist of 3 image container sptrs of slightly increasing size:
 * - 1x1
 * - 2x2
 * - 3x3
 */
ic_vec_t make_simple_ic_vec()
{
  using namespace kwiver::vital;

  using img_vec_t = std::vector< image_container_sptr >;
  img_vec_t img_vec;
  img_vec.push_back( std::make_shared< simple_image_container >( image( 1, 1 ) ) );
  img_vec.push_back( std::make_shared< simple_image_container >( image( 2, 2 ) ) );
  img_vec.push_back( std::make_shared< simple_image_container >( image( 3, 3 ) ) );
  return img_vec;
}

} // end namespace: anonymous

// ----------------------------------------------------------------------------
TEST( simple_image_container_set, empty )
{
  kwiver::vital::image_container_set_sptr empty_set;
  empty_set = std::make_shared<kwiver::vital::simple_image_container_set>();

  // Check size is reported as zero
  EXPECT_EQ(0, empty_set->size()) << "Set empty";
}

// ----------------------------------------------------------------------------
// Test construction with a non-empty vector of image_container_sptrs
TEST( simple_image_container_set, construct_nonempty )
{
  using namespace kwiver::vital;
  // Make a vector of empty images.
  ic_vec_t img_vec = make_simple_ic_vec();
  simple_image_container_set sics( img_vec );
}

// ----------------------------------------------------------------------------
// Test size return from default construction
TEST( simple_image_container_set, size_empty )
{
  using namespace kwiver::vital;
  simple_image_container_set sics;
  EXPECT_EQ( sics.size(), 0 );
}

// ----------------------------------------------------------------------------
// Test set non-const iteration
TEST( simple_image_container_set, expected_iteration )
{
  using namespace kwiver::vital;
  // Make a vector of empty images.
  ic_vec_t img_vec = make_simple_ic_vec();
  simple_image_container_set sics( img_vec );

  // We should be able to iterate 3 times with img_vec and the returns should
  // be equivalent to the direct vector iterator
  ic_vec_t::iterator                   vec_it = img_vec.begin();
  simple_image_container_set::iterator sic_it = sics.begin();

  LOG_INFO( test_logger, "Testing iter pos 0" );
  EXPECT_NE( vec_it, img_vec.end() );
  EXPECT_NE( sic_it, sics.end() );
  EXPECT_EQ( *sic_it, img_vec[0] );
  EXPECT_EQ( *sic_it, *vec_it );
  EXPECT_EQ( (*sic_it)->width(), 1 );
  EXPECT_EQ( (*sic_it)->height(), 1 );

  LOG_INFO( test_logger, "Incrementing to pos 1" );
  ++vec_it;
  ++sic_it;
  LOG_INFO( test_logger, "Testing iter pos 1" );
  EXPECT_NE( vec_it, img_vec.end() );
  EXPECT_NE( sic_it, sics.end() );
  EXPECT_EQ( *sic_it, img_vec[1] );
  EXPECT_EQ( *sic_it, *vec_it );
  EXPECT_EQ( (*sic_it)->width(), 2 );
  EXPECT_EQ( (*sic_it)->height(), 2 );

  LOG_INFO( test_logger, "Incrementing to pos 2" );
  ++vec_it;
  ++sic_it;
  LOG_INFO( test_logger, "Testing iter pos 2" );
  EXPECT_NE( vec_it, img_vec.end() );
  EXPECT_NE( sic_it, sics.end() );
  EXPECT_EQ( *sic_it, img_vec[2] );
  EXPECT_EQ( *sic_it, *vec_it );
  EXPECT_EQ( (*sic_it)->width(), 3 );
  EXPECT_EQ( (*sic_it)->height(), 3 );

  ++vec_it;
  ++sic_it;
  LOG_INFO( test_logger, "Testing end pos" );
  EXPECT_EQ( vec_it, img_vec.end() );
  EXPECT_EQ( sic_it, sics.end() );
}

// ----------------------------------------------------------------------------
// Test set const iteration
TEST( simple_image_container_set, expected_iteration_const )
{
  using namespace kwiver::vital;

  // Make a vector of empty images.
  ic_vec_t img_vec = make_simple_ic_vec();
  simple_image_container_set sics( img_vec );

  // We should be able to iterate 3 times with img_vec and the returns should
  // be equivalent to the direct vector iterator
  ic_vec_t::const_iterator                   vec_it = img_vec.begin();
  simple_image_container_set::const_iterator sic_it = sics.begin();

  LOG_INFO( test_logger, "Testing iter pos 0" );
  EXPECT_NE( vec_it, img_vec.end() );
  EXPECT_NE( sic_it, sics.end() );
  EXPECT_EQ( *sic_it, img_vec[0] );
  EXPECT_EQ( *sic_it, *vec_it );
  EXPECT_EQ( (*sic_it)->width(), 1 );
  EXPECT_EQ( (*sic_it)->height(), 1 );

  LOG_INFO( test_logger, "Incrementing to pos 1" );
  ++vec_it;
  ++sic_it;
  LOG_INFO( test_logger, "Testing iter pos 1" );
  EXPECT_NE( vec_it, img_vec.end() );
  EXPECT_NE( sic_it, sics.end() );
  EXPECT_EQ( *sic_it, img_vec[1] );
  EXPECT_EQ( *sic_it, *vec_it );
  EXPECT_EQ( (*sic_it)->width(), 2 );
  EXPECT_EQ( (*sic_it)->height(), 2 );

  LOG_INFO( test_logger, "Incrementing to pos 2" );
  ++vec_it;
  ++sic_it;
  LOG_INFO( test_logger, "Testing iter pos 2" );
  EXPECT_NE( vec_it, img_vec.end() );
  EXPECT_NE( sic_it, sics.end() );
  EXPECT_EQ( *sic_it, img_vec[2] );
  EXPECT_EQ( *sic_it, *vec_it );
  EXPECT_EQ( (*sic_it)->width(), 3 );
  EXPECT_EQ( (*sic_it)->height(), 3 );

  LOG_INFO( test_logger, "Incrementing to iter end" );
  ++vec_it;
  ++sic_it;
  LOG_INFO( test_logger, "Testing end pos" );
  EXPECT_EQ( vec_it, img_vec.end() );
  EXPECT_EQ( sic_it, sics.end() );
}

// ----------------------------------------------------------------------------
// Test that creating and iterating through multiple iterators does not affect
// each other.
TEST( simple_image_container_set, multiple_iterators )
{
  using namespace kwiver::vital;
  // Make a vector of empty images.
  ic_vec_t img_vec = make_simple_ic_vec();
  simple_image_container_set sics( img_vec );

  simple_image_container_set::iterator it1 = sics.begin();
  simple_image_container_set::iterator it2 = sics.begin();

  EXPECT_EQ( *it1, img_vec[0] );
  EXPECT_EQ( (*it1)->width(), 1 );
  EXPECT_NE( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[0] );
  EXPECT_EQ( (*it2)->width(), 1 );
  EXPECT_NE( it2, sics.end() );

  // Move one iterator forward two and the other just one.
  ++it1; ++it1;
  ++it2;
  EXPECT_EQ( *it1, img_vec[2] );
  EXPECT_EQ( (*it1)->width(), 3 );
  EXPECT_NE( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  // Make new iterator, which should point to the beginning.
  simple_image_container_set::iterator it3 = sics.begin();
  EXPECT_EQ( *it1, img_vec[2] );
  EXPECT_EQ( (*it1)->width(), 3 );
  EXPECT_NE( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( *it3, img_vec[0] );
  EXPECT_EQ( (*it3)->width(), 1 );
  EXPECT_NE( it3, sics.end() );

  // Only move the newest iterator forward one.
  ++it3;
  EXPECT_EQ( *it1, img_vec[2] );
  EXPECT_EQ( (*it1)->width(), 3 );
  EXPECT_NE( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( *it3, img_vec[1] );
  EXPECT_EQ( (*it3)->width(), 2 );
  EXPECT_NE( it3, sics.end() );

  // Move it1 forward to end.
  ++it1;
  EXPECT_EQ( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( *it3, img_vec[1] );
  EXPECT_EQ( (*it3)->width(), 2 );
  EXPECT_NE( it3, sics.end() );

  // Move it3 to end.
  ++it3; ++it3;
  EXPECT_EQ( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( it3, sics.end() );

  // Move it2 forward one.
  ++it2;
  EXPECT_EQ( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[2] );
  EXPECT_EQ( (*it2)->width(), 3 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( it3, sics.end() );

  // Move it2 to end
  ++it2;
  EXPECT_EQ( it1, sics.end() );
  EXPECT_EQ( it2, sics.end() );
  EXPECT_EQ( it3, sics.end() );
}

// ----------------------------------------------------------------------------
// Test that creating and iterating through multiple iterators does not affect
// each other. (const version)
TEST( simple_image_container_set, multiple_iterators_const )
{
  using namespace kwiver::vital;
  // Make a vector of empty images.
  ic_vec_t img_vec = make_simple_ic_vec();
  simple_image_container_set sics( img_vec );

  simple_image_container_set::const_iterator it1 = sics.begin();
  simple_image_container_set::const_iterator it2 = sics.begin();

  EXPECT_EQ( *it1, img_vec[0] );
  EXPECT_EQ( (*it1)->width(), 1 );
  EXPECT_NE( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[0] );
  EXPECT_EQ( (*it2)->width(), 1 );
  EXPECT_NE( it2, sics.end() );

  // Move one iterator forward two and the other just one.
  ++it1; ++it1;
  ++it2;
  EXPECT_EQ( *it1, img_vec[2] );
  EXPECT_EQ( (*it1)->width(), 3 );
  EXPECT_NE( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  // Make new iterator, which should point to the beginning.
  simple_image_container_set::iterator it3 = sics.begin();
  EXPECT_EQ( *it1, img_vec[2] );
  EXPECT_EQ( (*it1)->width(), 3 );
  EXPECT_NE( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( *it3, img_vec[0] );
  EXPECT_EQ( (*it3)->width(), 1 );
  EXPECT_NE( it3, sics.end() );

  // Only move the newest iterator forward one.
  ++it3;
  EXPECT_EQ( *it1, img_vec[2] );
  EXPECT_EQ( (*it1)->width(), 3 );
  EXPECT_NE( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( *it3, img_vec[1] );
  EXPECT_EQ( (*it3)->width(), 2 );
  EXPECT_NE( it3, sics.end() );

  // Move it1 forward to end.
  ++it1;
  EXPECT_EQ( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( *it3, img_vec[1] );
  EXPECT_EQ( (*it3)->width(), 2 );
  EXPECT_NE( it3, sics.end() );

  // Move it3 to end.
  ++it3; ++it3;
  EXPECT_EQ( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[1] );
  EXPECT_EQ( (*it2)->width(), 2 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( it3, sics.end() );

  // Move it2 forward one.
  ++it2;
  EXPECT_EQ( it1, sics.end() );

  EXPECT_EQ( *it2, img_vec[2] );
  EXPECT_EQ( (*it2)->width(), 3 );
  EXPECT_NE( it2, sics.end() );

  EXPECT_EQ( it3, sics.end() );

  // Move it2 to end
  ++it2;
  EXPECT_EQ( it1, sics.end() );
  EXPECT_EQ( it2, sics.end() );
  EXPECT_EQ( it3, sics.end() );
}
