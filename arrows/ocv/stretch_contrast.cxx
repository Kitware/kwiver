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
 * \brief Implementation of image contrast stretching
 */

#include "stretch_contrast.h"

#include <vital/exceptions.h>
#include <vital/util/wall_timer.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/imgproc/imgproc.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;

static
cv::Mat
cumsum( cv::Mat &src )
{
  cv::Mat result = cv::Mat::zeros( cv::Size( src.cols, src.rows ), CV_32FC1 );
  for (int i = 0; i < src.rows; ++i)
  {
     for (int j = 0; j < src.cols; ++j)
     {
        if (i == 0)
        {
            result.at<float>(i, j) = src.at<float>(i, j);
         }
         else
         {
             result.at<float>(i, j) = src.at<float>(i, j) + result.at<float>(i-1, j);
         }
       }
    }
return result;
}


static
std::vector<float>
string_to_vector( std::string list )
{
  std::vector<float> vect;
  std::stringstream ss( list );
  float f;
  while( ss >> f )
  {
    vect.push_back(f);
    if( ss.peek() == ',' )
    {
      ss.ignore();
    }
  }
  return vect;
}


/// Private implementation class
class stretch_contrast::priv
{
public:
  enum color_handling_modes { all_separately, luminance } ;
  color_handling_modes m_color_mode=all_separately;
  std::vector<float> m_from_percentiles;
  std::vector<float> m_to_percentiles;
  kwiver::vital::logger_handle_t m_logger;
  kwiver::vital::wall_timer m_timer;

  /// Constructor
  priv()
  {
  }

  /// Destructor
  ~priv()
  {
  }

  void
  set_color_handling(const std::string color_mode)
  {
    if( color_mode == "all_separately" )
    {
      m_color_mode = all_separately;
    }
    else if( color_mode == "luminance" )
    {
      m_color_mode = luminance;
    }
    else
    {
      throw vital::invalid_data( "color_mode '" + color_mode + "' not recognized." );
    }
  }


  void
  stretch_contrast(const cv::Mat &src, cv::Mat &dst)
  {
    if( src.channels() == 1 )
    {
      // Number of bins for the histogram
      int hist_size = 256;
      float range[2] = { 0, 256 } ;

      if( src.type() == CV_8UC1 )
      {
        // Maximum intensity value possible for the data type
        range[1] = 256;
      }
      else
      {
        throw vital::invalid_data( "Only 8-bit imagery is supported at this "
                                   "time" );
      }
      
      cv::Mat hist;
      const float* hist_range = { range };
      cv::calcHist( &src, 1, 0, cv::Mat(), hist, 1, &hist_size, &hist_range,
                    true, false );
      hist.convertTo( hist, CV_32FC1 );
      cv::Mat hist_cumsum = cumsum( hist );

      // hist_cumsum provides the cumulative histogram with the ith element
      // being the number of intensity instances up to and including the ith
      // bin. Hence, the last element of hist_cumsum is one.
      hist_cumsum = hist_cumsum / hist_cumsum.at<float>(hist_cumsum.rows-1, 0);

      // The histogram captures intensities instances with the defined intensity
      // bins. In a 0-1 floating point image intensity scale, the center of the
      // first bin corresponds to 1/hist_size/2 and the center of the last bin
      // corresponds to 1 - 1/hist_size/2. Converting from histogram bin index i
      // to floating-point image equivalent intensity: i*c1 + c2.
      float c2 = 1/hist_size/2;
      float c1 = (1 - 2*c2)/(hist_size-1);
      
      // Obtain the source image intensities corresponding to m_from_percentiles
      std::vector<float> src_intensities;
      float valj, valjm1, t, perc;
      for( size_t i=0; i < m_from_percentiles.size(); ++i )
      {
        perc = m_from_percentiles[i]/100;
        for( int j=0; j < hist_size; ++j )
        {
          valj = hist_cumsum.at<float>(j, 0);
          if( valj >= perc )
          {
            if( j == 0 )
            {
              src_intensities.push_back( j*c1 + c2 );
            }
            else
            {
              // Linear interpolation
              t = ( valj - perc )/( valj - valjm1 );
              src_intensities.push_back( (t*( j - 1 ) + ( 1 - t )*j)*c1 + c2 );
            }
            break;
          }
          valjm1 = valj;
        }
      }
      
      // Obtain the destination image intensities corresponding to
      // m_to_percentiles of the image data type's full range
      std::vector<float> dst_intensities;
      for( size_t i=0; i < m_to_percentiles.size(); ++i )
      {
        dst_intensities.push_back( m_to_percentiles[i]/100 );
      }
      
      // Convert src_intensities and dst_intensities from floating-point 0-1
      // range to the output data type.
      if( src.type() == CV_8UC1 )
      {
        for( size_t i=0; i < src_intensities.size(); ++i )
        {
          // With f being the (0-1) floating point value and u being the uint8
          // value, f = u*(1-1/256)/255 + 1/512 and u = 256*f - 1/2
          src_intensities[i] = src_intensities[i]*256 - 0.5;
          dst_intensities[i] = dst_intensities[i]*256 - 0.5;
        }
        
        // Build a lookup table that represents a piece-wise linear extension of
        // the mapping between percentile and intensities. Percentiles and
        // likewise src_intensities are monotonically increasing.
        cv::Mat lookup_table( 1, 256, CV_8U );
        uchar* p = lookup_table.ptr();
        
        float s1, s2, d1, d2, m, b;
        for( size_t i=0; i < src_intensities.size() - 1; ++i )
        {
          // Map the elements of lookup_table that correspond to source image
          // intensities between s1 and s2 to d1 to d2 according to a linear
          // fit.
          s1 = src_intensities[i];
          s2 = src_intensities[i+1];
          d1 = dst_intensities[i];
          d2 = dst_intensities[i+1];
          
          // Define line d = m*s + b mapping source intensity to destination
          // intensity.
          m = (d2 - d1)/(s2 - s1);
          b = d1 - s1*m;
          
          if( i == 0 )
          {
            s1 = 0;
          }
          else
          {
            s1 = std::ceil( s1 );
          }
          
          if( i == src_intensities.size() - 2 )
          {
            s2 = 255;
          }
          else
          {
            s2 = std::floor( s2 );
          }
          
          for( int s = s1; s <= s2 ; ++s)
          {
            // In this case, s is the source intensity under consideration.
            p[s] = cv::saturate_cast<uchar>( std::round( s*m + b ) );
          }
        }

        cv::LUT( src, lookup_table, dst );
      }
      else
      {
        throw vital::invalid_data( "Only 8-bit imagery is supported at this "
                                   "time" );
      }

    }
    // The following handles color images by decomposing to monochrome images
    // and calls stretch_contrast again with each monochrome sub-image
    else if( src.channels() == 3 )
    {
      switch( m_color_mode )
      {
        case all_separately:
        {
          // Each channel is equalized independently
          cv::Mat RGB_3[3];
          cv::split( src, RGB_3 );
          for( size_t i=0; i < 3; ++i)
          {
            stretch_contrast( RGB_3[i], RGB_3[i] );
          }
          cv::merge( RGB_3, 3, dst );
          break;
        }
        case luminance:
        {
          cv::Mat ycbcr, YCBCR[3];
          cv::cvtColor(src, ycbcr, CV_RGB2YCrCb, 3);
          cv::split( ycbcr, YCBCR );
          stretch_contrast( YCBCR[0], YCBCR[0] );
          //YCBCR[1] = cv::Scalar(123);   // for testing
          //YCBCR[2] = cv::Scalar(123);   // for testing
          cv::merge( YCBCR, 3, ycbcr );
          cv::cvtColor(ycbcr, dst, CV_YCrCb2RGB, 3);
          break;
        }
        default:
        {
          throw "Unrecognized color mode.";
        }
      }
    }
    else
    {
      throw vital::invalid_data( "Image must have 1 or 3 channels but instead "
                                 "had "  + std::to_string( src.channels() ));
    }
    return;
  }
};


/// Constructor
stretch_contrast
::stretch_contrast()
: d_(new priv)
{
  attach_logger( "arrows.ocv.stretch_contrast" );
  d_->m_logger = logger();
}

/// Destructor
stretch_contrast
::~stretch_contrast() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
stretch_contrast
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value( "color_mode", "all_separately",
                     "In the case of color images, this sets how the channels "
                     "are stretched. If set to 'all_separately', each channel "
                     "is equalized independently. If set to 'luminance', the "
                     "image is converted into YCbCr, the luminance is "
                     "equalized, and then the image is converted back to RGB." );
  config->set_value( "from_percentiles", "1,99", "Comma-separated list of "
                     "image value percentiles. For each image to be contrast "
                     "stretched, the pixel values associated with the "
                     "'from_percentiles' are calculated. A piece-wise linear "
                     "pixel-intensity transformation is calculated so that "
                     "these intensities are mapped to the associated "
                     "percentile of the data type's full range defined in "
                     "'to_percentiles'." );
  config->set_value( "to_percentiles", "1,99", "See documentation for "
                     "'from_percentiles'." );
  
  return config;
}


/// Set this algo's properties via a config block
void
stretch_contrast
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  std::string color_mode = config->get_value<std::string>("color_mode");
  d_->set_color_handling(color_mode);
  LOG_DEBUG( logger(), "Color mode: " + color_mode);
  
  std::string from_perc_str = config->get_value<std::string>("from_percentiles");
  std::string to_perc_str = config->get_value<std::string>("to_percentiles");
  
  LOG_DEBUG( logger(), "from_percentiles: " + from_perc_str );
  LOG_DEBUG( logger(), "to_percentiles: " + to_perc_str );
  
  d_->m_from_percentiles = string_to_vector( from_perc_str );
  d_->m_to_percentiles = string_to_vector( to_perc_str );
  
  if( d_->m_from_percentiles.size() != d_->m_to_percentiles.size() )
  {
    throw algorithm_configuration_exception( type_name(), impl_name(), 
                                             "Length of 'from_percentiles' "
                                             "must match 'to_percentiles'" );
  }
  
  for( size_t i=0; i < d_->m_from_percentiles.size() - 1; ++i )
  {
    if( d_->m_from_percentiles[i+1] < d_->m_from_percentiles[i] )
    {
      throw algorithm_configuration_exception( type_name(), impl_name(), 
                                               "'from_percentiles' must be a "
                                               "monotonically increasing, "
                                               "comma-separated list." );
    }
    if( d_->m_to_percentiles[i+1] < d_->m_to_percentiles[i] )
    {
      throw algorithm_configuration_exception( type_name(), impl_name(), 
                                               "'to_percentiles' must be a "
                                               "monotonically increasing, "
                                               "comma-separated list." );
    }
  }
}


bool
stretch_contrast
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}


/// Equalize an image's histogram
kwiver::vital::image_container_sptr
stretch_contrast::
filter( kwiver::vital::image_container_sptr img )
{
  if ( !img )
  {
    throw vital::invalid_data("Inputs to ocv::stretch_contrast are null");
  }

  LOG_TRACE( logger(), "Received image [" + std::to_string(img->width()) +
             ", " + std::to_string(img->height()) + ", " +
             std::to_string(img->depth()) + "]");

  cv::Mat cv_src {arrows::ocv::image_container::vital_to_ocv( img->get_image() )};
  cv::Mat cv_dest;

  if( cv_src.channels() == 1 )
  {
    // TODO: Figure out why this is necessary. Something is wrong with
    // vital_to_ocv for grayscale images.
    cv_src = cv_src.clone();
  }

  d_->stretch_contrast(cv_src, cv_dest);

  kwiver::vital::image_container_sptr image_dest;
  image_dest = std::make_shared<ocv::image_container>(cv_dest);

  return image_dest;
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
