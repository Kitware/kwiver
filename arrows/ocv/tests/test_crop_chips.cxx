/*ckwg +29
 * Copyright 2013-2016 by Kitware, Inc.
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
 * \brief test OCV image class
 */

#include <vital/plugin_loader/plugin_manager.h>

#include <vital/types/image_container.h>
#include <vital/types/image_container_set.h>

#include <arrows/ocv/crop_chips.h>
#include <arrows/ocv/image_container.h>
#include <arrows/ocv/image_io.h>

#include <gtest/gtest.h>


int
main(int argc, char* argv[])
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

using namespace kwiver::vital;

TEST(crop_chips, factory)
{
  using namespace kwiver::arrows;

  kwiver::vital::plugin_manager::instance().load_all_plugins();

  algo::crop_chips_sptr algo = kwiver::vital::algo::crop_chips::create("ocv");
  EXPECT_NE(nullptr, algo) << "Unable to create crop_chips algorithm of type ocv";
  algo::crop_chips* algo_ptr = algo.get();
  ASSERT_TRUE(typeid(*algo_ptr) == typeid(ocv::crop_chips)) << "Factory method did not construct the correct type";
}


namespace {

// TODO: The following functions create dummy image data for implementing
// tests. They have been copied from `kwiver/arrows/ocv/tests/test_image.cxx`.
// It would be useful to separate them out into a helpers file to avoid code
// duplication.

// helper function to populate the image with a pattern
// the dynamic range is stretched between minv and maxv
template <typename T>
void
populate_ocv_image(cv::Mat& img, T minv, T maxv)
{
  const double range = static_cast<double>(maxv) - static_cast<double>(minv);
  const double offset = - minv;
  const unsigned num_c = img.channels();
  for( unsigned int p=0; p<num_c; ++p )
  {
    for( unsigned int j=0; j<static_cast<unsigned int>(img.rows); ++j )
    {
      for( unsigned int i=0; i<static_cast<unsigned int>(img.cols); ++i )
      {
        const double pi = 3.14159265358979323846;
        double val = ((std::sin(pi*double(i)*(p+1)/10) * std::sin(pi*double(j)*(p+1)/10))+1) / 2;
        img.template ptr<T>(j)[num_c * i + p] = static_cast<T>(val * range + offset);
      }
    }
  }
}


// helper function to populate the image with a pattern
template <typename T>
void
populate_ocv_image(cv::Mat& img)
{
  const T minv = std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::min() : T(0);
  const T maxv = std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::max() : T(1);
  populate_ocv_image(img, minv, maxv);
}


// helper function to populate the image with a pattern
// the dynamic range is stretched between minv and maxv
template <typename T>
void
populate_vital_image(kwiver::vital::image& img, T minv, T maxv)
{
  const double range = static_cast<double>(maxv) - static_cast<double>(minv);
  const double offset = - minv;
  for( unsigned int p=0; p<img.depth(); ++p )
  {
    for( unsigned int j=0; j<img.height(); ++j )
    {
      for( unsigned int i=0; i<img.width(); ++i )
      {
        const double pi = 3.14159265358979323846;
        double val = ((std::sin(pi*double(i)*(p+1)/10) * std::sin(pi*double(j)*(p+1)/10))+1) / 2;
        img.at<T>(i,j,p) = static_cast<T>(val * range + offset);
      }
    }
  }
}


// helper function to populate the image with a pattern
template <typename T>
void
populate_vital_image(kwiver::vital::image& img)
{
  const T minv = std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::min() : T(0);
  const T maxv = std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::max() : T(1);
  populate_vital_image<T>(img, minv, maxv);
}


} // end anonymous namespace


TEST(crop_chips, simple)
{
  using namespace kwiver;
  using namespace kwiver::arrows;

  kwiver::vital::image_of<uint8_t> img(200,300,3);
  populate_vital_image<uint8_t>(img);

  image_container_sptr img_sptr(new simple_image_container(img));

  std::vector< kwiver::vital::bounding_box_d > bboxes0;

  // TODO: expand this
  ocv::crop_chips algo;
  auto output0 = algo.crop(img_sptr, bboxes0);
  EXPECT_EQ(0, bboxes0.size());
  EXPECT_EQ(0, output0->size());

  std::vector< kwiver::vital::bounding_box_d > bboxes3;
  bboxes3.push_back( kwiver::vital::bounding_box<double>( 1, 3, 10, 34 ) );
  bboxes3.push_back( kwiver::vital::bounding_box<double>( 10, 11, 40, 42 ) );
  bboxes3.push_back( kwiver::vital::bounding_box<double>( 5, 5, 5, 5 ) );
  auto output3 = algo.crop(img_sptr, bboxes3);

  EXPECT_EQ(3, bboxes3.size());
  EXPECT_EQ(3, output3->size());

  // TODO: test to ensure cropped sizes agree with bounding box width / heights.

  // TODO: add a test that checks that the pixel data hasn't changed.

  // TODO: add a cropping test when the bounding boxes coordinates contain
  // floating point non-integer numbers.
}
