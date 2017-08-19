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


#include <string>
//#include <boost/filesystem.hpp>

#include "stabilize_video_KLT.h"

#include <kwiversys/SystemTools.hxx>
#include <vital/exceptions.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/eigen.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


// ============================================================================
/// Private implementation class
class stabilize_video_KLT::priv
{
  cv::Mat m_key_frame_mono, m_moving_frame_mono, m_mask;
  int m_key_frame_type, m_rows, m_cols, m_precision;
  cv::Size m_raw_size, m_rendered_size;
  
public:
  std::string m_debug_dir;
  bool m_output_to_debug_dir = false;
  kwiver::vital::logger_handle_t m_logger;
  int m_max_pts;
  double m_pt_quality_thresh;
  double m_min_pt_dist;
  int m_patch_size;
  cv::TermCriteria m_termcrit;
  double m_reproj_thresh;
  double m_min_fract_pts;
  int m_max_disp;
  
  // Used to provide an index of the frame for debug out purposes
  int frame_index{-1};
  int key_frame_index{-1};
  
  std::vector<cv::Point2f> key_corners, moving_corners, final_moving_corners;
  
  /// Constructor
  /**
   * \param max_pts maximum number of key points to generate
   * \param pt_quality_thresh mimimum allowable feature point quality
   * \param min_pt_dist minimum distance between key points
   * \param reproj_thresh threshold error for robust homography fitting
   * \param min_fract_pts When the fraction of matched key points fall below 
   *    this value, a key frame update is triggered
   * \param max_disp number of pixels around the edge of the key frame to 
   *    use as a buffer in the rendered stabilized image
   */
  priv()
    : m_max_pts(5000),
      m_pt_quality_thresh(0.001), 
      m_min_pt_dist(10), 
      m_patch_size(101),
      m_termcrit(cv::TermCriteria(cv::TermCriteria::COUNT|cv::TermCriteria::EPS,200,0.05)),
      m_reproj_thresh(2), 
      m_min_fract_pts(0.1), 
      m_max_disp(50)
  {
  }
  
  /// Update the key frame and recalculate features to match to
  /**
   * \param _frame key frame to stabilize successive frames to
   * \param _mask mask to avoid key points
   */
  void
  update_key_frame(cv::InputArray frame_, cv::InputArray mask_=cv::Mat())
  {
    ++key_frame_index;
    
    LOG_TRACE( m_logger, "Updating key frame");
    if( false )
    {
      std::cout << "Updating key frame" << std::endl;
      std::cout << "max_pts : " << m_max_pts << std::endl;
      std::cout << "pt_quality_thresh : " << m_pt_quality_thresh << std::endl;
      std::cout << "min_pt_dist : " << m_min_pt_dist << std::endl;
      std::cout << "reproj_thresh : " << m_reproj_thresh << std::endl;
      std::cout << "min_fract_pts : " << m_min_fract_pts << std::endl;
      std::cout << "max_disp : " << m_max_disp << std::endl;
    }

    cv::Mat frame = frame_.getMat();

    m_rows = frame.rows, m_cols = frame.cols;
    m_raw_size = cv::Size(m_cols, m_rows);

    // The size of the rendered stabilized image
    m_rendered_size = cv::Size(m_cols-2*m_max_disp,m_rows-2*m_max_disp);

    if(mask_.kind() != cv::_InputArray::NONE)
    {
      m_mask = mask_.getMat();
    }

    if( frame.channels() == 3 )
    {
      std::cout << "Converting RGB key_frame to mono" << std::endl;
      cv::cvtColor(frame, m_key_frame_mono, CV_BGR2GRAY);
    }
    else
    {
      frame.copyTo(m_key_frame_mono);
    }

    cv::goodFeaturesToTrack(m_key_frame_mono, key_corners, m_max_pts, 
                            m_pt_quality_thresh, m_min_pt_dist, m_mask, 5, 
                            false, 0.04);
    moving_corners = key_corners;
    final_moving_corners = key_corners;
  }

  /// Measure against key frame and return homography to work back to key frame
  /**
   * \param [in] _moving_frame frame to stabilize
   * \returns homography matrix that warps points from the source to moving 
   *          back to the key frame. If the stabilization fails, 
   */
  cv::Mat
  measure_transform(cv::InputArray _moving_frame)
  {
    ++frame_index;
    
    cv::Mat moving_frame = _moving_frame.getMat();
    cv::Mat M;

    CV_Assert( moving_frame.rows == m_rows);
    CV_Assert( moving_frame.cols == m_cols);

    if( moving_frame.channels() == 3 )
    {
      cv::cvtColor(moving_frame, m_moving_frame_mono, CV_BGR2GRAY);
    }
    else
    {
      m_moving_frame_mono = moving_frame;
    }

    std::vector<uchar> status;
    std::vector<float> err;
    cv::Size win_size(m_patch_size,m_patch_size);
    cv::calcOpticalFlowPyrLK(m_key_frame_mono, m_moving_frame_mono, key_corners, 
                             moving_corners, status, err, win_size, 3, m_termcrit, 
                             cv::OPTFLOW_USE_INITIAL_FLOW,
                             0.001);

    int n = cv::sum(status)[0];
    std::cout << "calcOpticalFlowPyrLK found " << n << " matches out of " << 
            key_corners.size() << std::endl;

    if( n < key_corners.size()*m_min_fract_pts)
    {
      // Stabilization failed
      LOG_TRACE( m_logger, "Not enough corners were successfully tracked by "
                           "calcOpticalFlowPyrLK.");
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
    M = cv::findHomography(src_pts, dst_pts, cv::RANSAC, m_reproj_thresh, pt_mask);
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
      if( erri <= m_reproj_thresh )
      {
        src_pts[k] = moving_corners[i];
        dst_pts[k] = key_corners[i];
        ++ k;
        //std::cout << "ERROR is: " << erri << std::endl;
      }
    }
    src_pts.resize(k);
    dst_pts.resize(k);


    if( src_pts.size() < key_corners.size()*m_min_fract_pts)
    {
      // Stabilization failed
      LOG_TRACE( m_logger, "Not enough corners passed robust homography fitting");
      M.release();
      return M;
    }


    if( true )
    {
      // Fit a rigid transformation.
      std::cout << "Fitting rigid transformation" << std::endl;

      // Fit to a rigid transformation (i.e., translation plus rotation)
      std::array<double,3> threshes{4*m_reproj_thresh,2*m_reproj_thresh,1.5*m_reproj_thresh};
      for( int j = 0; j < 3; ++j)
      {
        M = cv::estimateRigidTransform(src_pts, dst_pts, false);

        if( M.empty() )
        {
          // Stabilization failed
          LOG_TRACE( m_logger, "Not enough corners passed rigid homography fitting");
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
          LOG_TRACE( m_logger, "Not enough corners passed rigid homography fitting");
          return M;
        }
        src_pts.resize(k);
        dst_pts.resize(k);
      }

      std::cout << "RANSAC Homography fitting matches: " << src_pts.size() << 
              " of " << key_corners.size() << std::endl;

      if( src_pts.size() < key_corners.size()*m_min_fract_pts)
      {
        // Stabilization failed
        M.release();
        LOG_TRACE( m_logger, "Not enough corners passed rigid homography fitting");
        return M;
      }
    }
    
    std::vector<cv::Point2f> rendered_corners(4), back_proj_corners(4);
    rendered_corners[0] = cvPoint(m_max_disp, 
                                  m_max_disp);
    rendered_corners[1] = cvPoint(m_raw_size.width - m_max_disp, 
                                  m_max_disp);
    rendered_corners[2] = cvPoint(m_raw_size.width - m_max_disp, 
                                  m_raw_size.height - m_max_disp);
    rendered_corners[3] = cvPoint(m_max_disp, 
                                  m_raw_size.height - m_max_disp);

    cv::perspectiveTransform(rendered_corners, back_proj_corners, M.inv());

    // Make sure that the rendered image will not deviate by more than max_disp
    // from the corners of the key frame.
    bool failed=false;
    cv::Point2f pt;
    double b = 4;   // interpolation buffer
    double w=m_raw_size.width, h=m_raw_size.height;
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
      LOG_TRACE( m_logger, "Frame moved too much.");
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

  /// Has a key frame already been estabilished
  bool
  has_key_frame()
  {
    if(m_key_frame_mono.empty() == cv::_InputArray::NONE)
    {
      // No key frame exists
      return false;
    }
    return true;
  }
  
  /// Set up debug directory
  void
  setup_debug_dir()
  {
    LOG_DEBUG( m_logger, "Creating debug directory: " + m_debug_dir);
    kwiversys::SystemTools::MakeDirectory(m_debug_dir);
    m_output_to_debug_dir = true;
  }
  
  /// Save frame with annotated key points to debug directory
  void
  save_frame_debug_dir(const cv::Mat& frame0, const cv::Mat& H, 
                       const timestamp& ts)
  {
    // Warp current frame's key points into the stabilized version of that frame
    std::vector< cv::Point2f > corners;
    cv::perspectiveTransform(final_moving_corners, corners, H);
    
    // Warp frame into stabilized coordinate system
    cv::Mat image;
    int flag = cv::INTER_LINEAR;
    cv::warpPerspective(frame0, image, H, m_rendered_size, 
                        flag);
    cv::cvtColor(image, image, CV_BGR2RGB);
    
    // Superimpose key points
    for( size_t j = 0; j < corners.size(); j++ )
    {
      cv::circle(image, corners[j], 3, cv::Scalar(0, 255, 255), 1);
    }
    
    cv::putText(image, "Key Frame " + std::to_string(key_frame_index), 
            cvPoint(30,30), cv::FONT_HERSHEY_PLAIN, 2, cvScalar(0,255,0), 
            1, CV_AA);
    
    std::string frame_time = std::to_string(ts.get_time_usec());
    imwrite(m_debug_dir + "/" + std::to_string(frame_index) + ".tif", image);
  }
};
// ============================================================================


// ----------------------------------------------------------------------------
/// Constructor
stabilize_video_KLT
::stabilize_video_KLT()
: d_(new priv())
{
  attach_logger( "arrows.ocv.stabilize_video_KLT" );
  d_->m_logger = logger();
}


// ----------------------------------------------------------------------------
/// Destructor
stabilize_video_KLT
::~stabilize_video_KLT() VITAL_NOTHROW
{
}


// ----------------------------------------------------------------------------
/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
stabilize_video_KLT
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();
  
  config->set_value( "max_disp", d_->m_max_disp,
                     "Number of pixels of camera motion that will trigger a "
                     "key frame update." );
  config->set_value( "max_pts", d_->m_max_pts,
                     "Maximum number of features to detect in the key frame.  "
                     "See maxCorners in OpenCV goodFeaturesToTrack." );
  config->set_value( "pt_quality_thresh", d_->m_pt_quality_thresh,
                     "The minimal accepted quality of key frame features.  "
                     "See qualityLevel in OpenCV goodFeaturesToTrack." );
  config->set_value( "min_pt_dist", d_->m_min_pt_dist,
                     "Minimum distance between features in the key frame.  "
                     "See minDistance in OpenCV goodFeaturesToTrack." );
  config->set_value( "patch_size", d_->m_patch_size,
                     "Minimum distance between features in the key frame.  "
                     "See minDistance in OpenCV goodFeaturesToTrack." );
  config->set_value( "reproj_thresh", d_->m_reproj_thresh,
                     "Robust homography fitting reprojection error threshold." );
  config->set_value( "min_fract_pts", d_->m_min_fract_pts,
                     "When the fraction of tracked points that pass the robust "
                     "homography fitting threshold falls below this threshold, "
                     "a key frame update will be triggered.");
  config->set_value( "debug_dir", d_->m_debug_dir,
                     "Output debug images to this directory.");
  return config;
}


// ----------------------------------------------------------------------------
/// Set this algo's properties via a config block
void
stabilize_video_KLT
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);
  
  d_->m_max_disp          = config->get_value<int>( "max_disp" );
  d_->m_max_pts           = config->get_value<int>( "max_pts" );
  d_->m_pt_quality_thresh = config->get_value<double>( "pt_quality_thresh" );
  d_->m_min_pt_dist       = config->get_value<double>( "min_pt_dist" );
  d_->m_patch_size        = config->get_value<double>( "patch_size" );
  d_->m_min_fract_pts     = config->get_value<double>( "min_fract_pts" );
  d_->m_reproj_thresh     = config->get_value<double>( "reproj_thresh" );
  d_->m_debug_dir         = config->get_value<std::string>( "debug_dir" );
  
  if ( ~(d_->m_debug_dir.empty()) )
  {
    d_->setup_debug_dir();
  }
}


// ----------------------------------------------------------------------------
bool
stabilize_video_KLT
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}


// ----------------------------------------------------------------------------
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
  
  if(d_->has_key_frame())
  {
    // No key frame exists
    d_->update_key_frame(cv_src);
  }
    
  //cv::Mat H_cv = (cv::Mat_<double>(3,3) << 2, 0, 0, 0, 1, 0, 0, 0, 1);
  
  cv::Mat H_cv = d_->measure_transform(cv_src);
  
  if( H_cv.empty() )
    {
      // Stabilization failed
      d_->update_key_frame(cv_src);
      H_cv = cv::Mat::eye(3, 3, CV_64F);
      coordinate_system_updated = true;
    }
  else
  {
    coordinate_system_updated = false;
  }
  
  if( d_->m_output_to_debug_dir )
  {
    d_->save_frame_debug_dir(cv_src, H_cv, ts);
  }
  
  vital::matrix_3x3d H_mat;
  cv2eigen(H_cv, H_mat);
  src_to_ref = homography_f2f_sptr(new homography_f2f(H_mat, ts, ts));
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
