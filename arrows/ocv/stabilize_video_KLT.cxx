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

/**
 * \file
 * \brief Implementation of ocv::stabilize_video_KLT
 */

#include "stabilize_video_KLT.h"

#include <vital/exceptions.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/eigen.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


//-----------------------------------------------------------------------------
klt_stabilizer
::klt_stabilizer(int _max_disp, int _max_corners, double _quality_level, 
                 double _min_distance, double _min_fract_pts)
{
  max_disp = _max_disp;
  max_corners = _max_corners;
  quality_level = _quality_level;
  min_distance = _min_distance;
  termcrit = cv::TermCriteria(cv::TermCriteria::COUNT|cv::TermCriteria::EPS,200,0.05);
  reproj_thresh = 2;
  min_fract_pts = _min_fract_pts;
    
  //this->update_key_frame(_frame);
}


//-----------------------------------------------------------------------------
void
klt_stabilizer
::update_key_frame(cv::InputArray _frame, cv::InputArray _mask)
{
  std::cout << "Updating key frame" << std::endl;
  cv::Mat frame = _frame.getMat();
  
  rows = frame.rows, cols = frame.cols;
  raw_size = cv::Size(cols, rows);
  
  // The size of the rendered stabilized image
  rendered_size = cv::Size(cols-2*max_disp,rows-2*max_disp);
  
  if(_mask.kind() != cv::_InputArray::NONE)
  {
    mask = _mask.getMat();
  }
    
  if( frame.channels() == 3 )
  {
    std::cout << "Converting RGB key_frame to mono" << std::endl;
    cv::cvtColor(frame, key_frame_mono, CV_BGR2GRAY);
  }
  else
  {
    frame.copyTo(key_frame_mono);
  }
  
  cv::goodFeaturesToTrack(key_frame_mono, key_corners, max_corners, 
                          quality_level, min_distance, mask, 5, false, 0.04);
  moving_corners = key_corners;
  final_moving_corners = key_corners;
}


//-----------------------------------------------------------------------------
cv::Mat
klt_stabilizer
::measure_transform(cv::InputArray _moving_frame)
{
  cv::Mat moving_frame = _moving_frame.getMat();
  cv::Mat M;
  
  CV_Assert( moving_frame.rows == rows);
  CV_Assert( moving_frame.cols == cols);
  
  if( moving_frame.channels() == 3 )
  {
    cv::cvtColor(moving_frame, moving_frame_mono, CV_BGR2GRAY);
  }
  else
  {
    moving_frame_mono = moving_frame;
  }
  
  std::vector<uchar> status;
  std::vector<float> err;
  cv::Size win_size(101,101);
  cv::calcOpticalFlowPyrLK(key_frame_mono, moving_frame_mono, key_corners, 
                           moving_corners, status, err, win_size, 3, termcrit, 
                           cv::OPTFLOW_USE_INITIAL_FLOW,
                           0.001);
  
  int n = cv::sum(status)[0];
  std::cout << "calcOpticalFlowPyrLK found " << n << " matches out of " << 
          key_corners.size() << std::endl;
  
  if( n < key_corners.size()*min_fract_pts)
  {
    // Stabilization failed
    M.release();
    return M;
  }

  std::vector<cv::Point2f> src_pts(n);
  std::vector<cv::Point2f> dst_pts(n);

  int k = 0;
  for( unsigned i = 0; i < status.size(); ++i )
  {
    // Select only the inliers (mask entry set to 1)
    if (status[i] == 1)
    {
      //std::cout << "KLT Error: " << err[i] << std::endl;
      //std::cout << i << ", ";
      //std::cout << moving_corners[i] << std::endl;
      src_pts[k] = moving_corners[i];
      dst_pts[k] = key_corners[i];
      ++ k;
    }
  }
  
  final_moving_corners = src_pts;
  
  cv::Mat pt_mask;
  M = cv::findHomography(src_pts, dst_pts, cv::RANSAC, reproj_thresh, pt_mask);
  n = cv::sum(pt_mask)[0];
  
  // Transform points with the homography that was just fit
  std::vector<cv::Point2f> tformed_pts;
  cv::perspectiveTransform(src_pts, tformed_pts, M);

  k = 0;
  double erri;
  for( unsigned i = 0; i < src_pts.size(); i++ )
  {
    // Check that the errors are correctly below reproj_thresh
    erri = cv::norm(cv::Mat(tformed_pts[i]), cv::Mat(dst_pts[i]));
    if( erri <= reproj_thresh )
    {
      src_pts[k] = moving_corners[i];
      dst_pts[k] = key_corners[i];
      ++ k;
      //std::cout << "ERROR is: " << erri << std::endl;
    }
  }
  src_pts.resize(k);
  dst_pts.resize(k);
  
  
  if( src_pts.size() < key_corners.size()*min_fract_pts)
  {
    // Stabilization failed
    M.release();
    return M;
  }
  
  
  if( true )
  {
    // Fit a rigid transformation.
    std::cout << "Fitting rigid transformation" << std::endl;

    // Fit to a rigid transformation (i.e., translation plus rotation)
    std::array<double,3> threshes{4*reproj_thresh,2*reproj_thresh,1.5*reproj_thresh};
    for( int j = 0; j < 3; ++j)
    {
      M = cv::estimateRigidTransform(src_pts, dst_pts, false);

      if( M.empty() )
      {
        // Stabilization failed
        M.release();
        return M;
      }

      // Make a 3x3 homography
      cv::Mat new_row = (cv::Mat_<double>(1,3) << 0, 0, 1);
      M.push_back(new_row);

      // Check errors
      cv::perspectiveTransform(src_pts, tformed_pts, M);
      k = 0;
      double erri;
      for( unsigned i = 0; i < src_pts.size(); i++ )
      {
        // Check that the errors are correctly below reproj_thresh
        erri = cv::norm(cv::Mat(tformed_pts[i]), cv::Mat(dst_pts[i]));
        if( erri <= threshes[j] )
        {
          src_pts[k] = src_pts[i];
          dst_pts[k] = dst_pts[i];
          ++ k;
          //std::cout << "ERROR is: " << erri << std::endl;
        }
      }
      if( k == 0 )
      {
        // Stabilization failed
        M.release();
        return M;
      }
      src_pts.resize(k);
      dst_pts.resize(k);
    }

    std::cout << "RANSAC Homography fitting matches: " << src_pts.size() << 
            " of " << key_corners.size() << std::endl;
    
    if( src_pts.size() < key_corners.size()*min_fract_pts)
    {
      // Stabilization failed
      M.release();
      return M;
    }
  }
  
  // Accommodate the cropping so that we have a buffer for motion
  M.at<double>(0,2) -= max_disp;
  M.at<double>(1,2) -= max_disp;
  
  std::vector<cv::Point2f> rendered_corners(4), back_proj_corners(4);
  rendered_corners[0] = cvPoint(0,0);
  rendered_corners[1] = cvPoint(rendered_size.width, 0);
  rendered_corners[2] = cvPoint(rendered_size.width, rendered_size.height);
  rendered_corners[3] = cvPoint(0, rendered_size.height);
  
  cv::perspectiveTransform(rendered_corners, back_proj_corners, M.inv());
  
  // Make sure that the rendered image will not have any black in it.
  bool failed=false;
  cv::Point2f pt;
  double b = 4;
  double w=raw_size.width, h=raw_size.height;
  for( int i = 0; i < 4; i++ )
  {
    pt = back_proj_corners[i];
    //std::cout << "back_proj_corners: " << pt << std::endl;
    if( pt.x < b || pt.x > w-b || pt.y < b || pt.y > h-b)
    {
      failed = true;
      //std::cout << "Point: " << pt << std::endl;
    }
  }
  
  if( failed )
  {
    std::cout << "Frame moved too much" << std::endl;
    // Stabilization failed
    M.release();
    return M;
  }
  
  //std::cout << "Homography: " << M << std::endl;
  if( true )
  {
    final_moving_corners = src_pts;
  }
  
  return M;
}


//-----------------------------------------------------------------------------
bool
klt_stabilizer
::has_key_frame()
{
  if(key_frame_mono.empty() == cv::_InputArray::NONE)
  {
    // No key frame exists
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// ------------------------------ Sprokit ------------------------------------


/// Private implementation class
class stabilize_video_KLT::priv
{
public:
  // Create the stabilizer from the key frame.
  int max_disp = 50;
  int max_features = 5000;
  double quality_level = 0.001;
  double min_distance = 60;
  double min_fract_pts = 0.1;
  klt_stabilizer stab;
  
  /// Constructor
  priv()
     : stab(max_disp, max_features, quality_level, min_distance, min_fract_pts)
  {
  }
};


/// Constructor
stabilize_video_KLT
::stabilize_video_KLT()
: d_(new priv)
{
  attach_logger( "arrows.ocv.stabilize_video_KLT" );
}


/// Destructor
stabilize_video_KLT
::~stabilize_video_KLT() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
stabilize_video_KLT
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  return config;
}


/// Set this algo's properties via a config block
void
stabilize_video_KLT
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);
}


bool
stabilize_video_KLT
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}


/// Return homography to stabilize the image_src relative to the key frame 
void
stabilize_video_KLT
::process_image( const timestamp& ts,
                 const image_container_sptr image_src,
                 homography_f2f_sptr& src_to_ref,
                 bool&  coordinate_system_updated)
{
  if ( !image_src)
  {
    throw vital::invalid_data("Inputs to ocv::stabilize_video_KLT are null");
  }

  cv::Mat cv_src = ocv::image_container::vital_to_ocv(image_src->get_image());
  
  if(d_->stab.has_key_frame())
  {
    // No key frame exists
    d_->stab.update_key_frame(cv_src);
  }
    
  //cv::Mat H_cv = (cv::Mat_<double>(3,3) << 2, 0, 0, 0, 1, 0, 0, 0, 1);
  
  cv::Mat H_cv = d_->stab.measure_transform(cv_src);
  
  vital::matrix_3x3d H_mat;
  cv2eigen(H_cv, H_mat);
  src_to_ref = homography_f2f_sptr(new homography_f2f(H_mat, ts, ts));
  
  coordinate_system_updated = false;
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
