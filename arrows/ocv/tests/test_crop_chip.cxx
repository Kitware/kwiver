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

#include <test_common.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <arrows/ocv/image_container.h>
#include <arrows/ocv/image_container_set.h>
#include <arrows/ocv/crop_chips.h>

#define TEST_ARGS ()

DECLARE_TEST_MAP();

int
main(int argc, char* argv[])
{
  CHECK_ARGS(1);

  testname_t const testname = argv[1];

  RUN_TEST(testname);
}

using namespace kwiver::vital;

IMPLEMENT_TEST(factory)
{
  using namespace kwiver::arrows;

  kwiver::vital::plugin_manager::instance().load_all_plugins();

  //algo::crop_chips_sptr algo = kwiver::vital::algo::crop_chips_sptr::create("ocv");
  auto algo = kwiver::vital::algo::crop_chips_sptr::create("ocv");
  if (!algo)
  {
    TEST_ERROR("Unable to create crop_chips algorithm of type ocv");
  }
  algo::crop_chips* algo_ptr = algo.get();
  if (typeid(*algo_ptr) != typeid(ocv::crop_chips))
  {
    TEST_ERROR("Factory method did not construct the correct type");
  }
}


namespace {

// TODO: add these as general test helpers

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


IMPLEMENT_TEST(test_crop_simple)
{
  using namespace kwiver;
  using namespace kwiver::arrows;

  auto type_str = "uint8";
  std::cout << "Testing single channel cv::Mat of type " << type_str << std::endl;
  cv::Mat_<T> img(100,200);
  populate_ocv_image<T>(img);

  image_container_sptr img_sptr(new simple_image_container(img));

  std::vector< kwiver::vital::bounding_box_d > bboxes;

  // TODO: expand this
  ocv::image_io crop_chips;
  auto output = crop_chips.crop(img_sptr, bboxes);
}
