/*ckwg +29
 * Copyright 2013-2017 by Kitware, Inc.
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

/**
 * \file
 * \brief test OCV descriptor_set class
 */

#include <arrows/ocv/descriptor_set.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;
using namespace kwiver::arrows;

struct byte {}; // This is just a tag type that will show in the test name

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(descriptor_set, default_set)
{
  ocv::descriptor_set ds;
  EXPECT_EQ( 0, ds.size() );
  EXPECT_TRUE( ds.empty() );
  // Should not iterate at all
  ocv::descriptor_set::iterator it = ds.begin();
  size_t count = 0;
  for( const descriptor_sptr& d : ds )
  {
    (void)d; // avoid unused variable warning.
    ++count;
  }
  EXPECT_EQ( count, 0 );
}

// ----------------------------------------------------------------------------
TEST(descriptor_set, populated_set)
{
  static constexpr unsigned num_desc = 100;
  static constexpr unsigned dim = 128;

  cv::Mat data(num_desc, dim, CV_64F);
  cv::randu(data, 0, 1);
  ocv::descriptor_set ds(data);

  EXPECT_EQ( num_desc,  ds.size() );
  EXPECT_FALSE( ds.empty() );
  EXPECT_EQ( data.data, ds.ocv_desc_matrix().data )
    << "descriptor_set should contain original cv::Mat";

  // Iteration yield count should match expected size.
  size_t count = 0;
  ocv::descriptor_set::iterator it = ds.begin();
  for( const descriptor_sptr & d_sptr : ds )
  {
    (void)d_sptr; // avoid unused variable warning.
    ++count;
  }
  EXPECT_EQ( ds.size(), count );

  for ( unsigned i = 0; i < num_desc; ++i )
  {
    SCOPED_TRACE( "At descriptor " + std::to_string(i) );
    EXPECT_EQ( dim, ds.at(i)->size() );

    std::vector<double> vals = ds.at(i)->as_double();
    cv::Mat row = data.row(i);
    EXPECT_TRUE( std::equal( vals.begin(), vals.end(), row.begin<double>() ) );
  }
}

// ----------------------------------------------------------------------------
// Test spawning multiple iterators and that their returns do not conflict with
// each other.
TEST( descriptor_set, coiteration )
{
}

namespace {

// ----------------------------------------------------------------------------
void test_conversions(const cv::Mat& data)
{
  SCOPED_TRACE( "Data size: " + std::to_string( data.rows ) + "x" +
                                std::to_string( data.cols ) );

  ocv::descriptor_set ds(data);
  EXPECT_EQ( data.rows, static_cast<int>( ds.size() ) );

  // Iteration yield count should match expected size.
  size_t count = 0;
  ocv::descriptor_set::iterator it = ds.begin();
  for( const descriptor_sptr & d_sptr : ds )
  {
    (void)d_sptr; // avoid unused variable warning.
    ++count;
  }
  EXPECT_EQ( ds.size(), count );

  cv::Mat double_data;
  data.convertTo(double_data, CV_64F);

  [&]{
    for ( unsigned i = 0; i < ds.size(); ++i )
    {
      SCOPED_TRACE( "At descriptor " + std::to_string(i) );
      EXPECT_EQ( data.cols, static_cast<int>( ds.at(i)->size() ) );

      auto const& vals = ds.at(i)->as_double();
      auto const& byte_vals = ds.at(i)->as_bytes();
      EXPECT_EQ( ds.at(i)->num_bytes(), byte_vals.size() );

      cv::Mat row = double_data.row(i);
      EXPECT_TRUE( std::equal(vals.begin(), vals.end(), row.begin<double>() ) );
    }
  }();

  simple_descriptor_set simp_ds(
      std::vector<descriptor_sptr>( ds.begin(), ds.end() ) );
  cv::Mat recon_mat = ocv::descriptors_to_ocv_matrix(simp_ds);
  EXPECT_NE( data.data, recon_mat.data )
    << "Reconstructed matrix should point to new memory, not original";
  EXPECT_EQ( data.type(), recon_mat.type() );
  EXPECT_EQ( data.size(), recon_mat.size() );
  EXPECT_EQ( 0, cv::countNonZero( recon_mat != data ) );
}

// ----------------------------------------------------------------------------
template <typename T> cv::Mat rand_mat(int r, int c);

// ----------------------------------------------------------------------------
template <> inline cv::Mat rand_mat<double>(int r, int c)
{
  cv::Mat m(r, c, CV_64F);
  cv::randu(m, 0.0, 1.0);
  return m;
}

// ----------------------------------------------------------------------------
template <> inline cv::Mat rand_mat<float>(int r, int c)
{
  cv::Mat m(r, c, CV_32F);
  cv::randu(m, 0.0f, 1.0f);
  return m;
}

// ----------------------------------------------------------------------------
template <> inline cv::Mat rand_mat<::byte>(int r, int c)
{
  cv::Mat m(r, c, CV_8U);
  cv::randu(m, 0, 255);
  return m;
}

}

// ----------------------------------------------------------------------------
template <typename T>
class descriptor_set_conversion : public ::testing::Test
{
};

using conversion_types =
  ::testing::Types<::byte, float, double>;
TYPED_TEST_CASE(descriptor_set_conversion, conversion_types);

// ----------------------------------------------------------------------------
TYPED_TEST(descriptor_set_conversion, conversion)
{
  test_conversions( rand_mat<TypeParam>( 1,   50 ) );
  test_conversions( rand_mat<TypeParam>( 64,  50 ) );
  test_conversions( rand_mat<TypeParam>( 128, 1  ) );
  test_conversions( rand_mat<TypeParam>( 125, 20 ) );
  test_conversions( rand_mat<TypeParam>( 256, 10 ) );
}
