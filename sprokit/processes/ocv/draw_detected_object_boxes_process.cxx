/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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
 * \brief Implementation of draw_detected_object_boxes_process
 */

#include "draw_detected_object_boxes_process.h"

#include <vital/vital_types.h>
#include <vital/vital_foreach.h>
#include <vital/util/wall_timer.h>

#include <arrows/ocv/image_container.h>

#include <sprokit/processes/kwiver_type_traits.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <Eigen/Core>

#include <sstream>
#include <iostream>

namespace kwiver {

// ----------------------------------------------------------------
/**
 * \class draw_detected_object_boxes_process
 *
 * \brief Draws boxes around detected objects.
 *
 * \iports
 *
 * \iport{detected_object_set} List of detections to draw.
 *
 * \iport{image} Input image where boxes are drawn.
 *
 * \oports
 *
 * \oport{image} Updated image with boxes and other annotations.
 *
 * \configs
 *
 * \config{threshold} Min probability threshold for drawing
 * detections. Detections at and above this threshold are drawn. This
 * value is a float.
 *
 * \config{alpha_blend_prob} If this item is set to \b true, then
 * detections with a lower probability are drawn with more
 * transparency.
 *
 * \config{default_color}
 *
 */

// Constant for offsetting drawn labels
static const int multi_label_offset(15);

typedef  Eigen::Matrix< unsigned int, 3, 1 > ColorVector;

create_config_trait( threshold, float, "-1", "min probability for output (float)" );
create_config_trait( alpha_blend_prob, bool, "true", "If true, those who are less likely will be more transparent." );
create_config_trait( default_line_thickness, float, "1", "The default line thickness for a class" );
create_config_trait( default_color, std::string, "255 0 0", "The default color for a class (BGR)" );
create_config_trait( custom_class_color, std::string, "",
                     "List of class/thickness/color separated by semi-colon. For example: person/3/255 0 0;car/2/0 255 0" );
create_config_trait( ignore_file, std::string, "__background__", "List of classes to ignore, separated by semi-colon." );
create_config_trait( text_scale, float, "0.4", "the scale for the text label" );
create_config_trait( text_thickness, float, "1.0", "the thickness for text" );
create_config_trait( file_string, std::string, "", "If not empty, use this as a formated string to write output (i.e. out_%5d.png)" );
create_config_trait( clip_box_to_image, bool, "false", "make sure the bounding box is only in the image" );
create_config_trait( draw_text, bool, "true", "Draw the text" );
create_config_trait( merge_overlapping_classes, bool, "true", "Combine overlapping classes" );
create_config_trait( draw_other_classes, bool, "false", "Print all combined overlap" );


/**
 * @brief
 *
 * Note that the detections in the input set are modified.
 *
 * @param input_set
 * @param ignore_classes
 *
 * @return A combined set of detections
 */
vital::detected_object_set_sptr
NMS_COMBINER( vital::detected_object_set_sptr input_set,
              const std::vector< std::string >&     ignore_classes )
{
  // Get a copy of the detections
  auto det_list = input_set->select();
  kwiver::vital::detected_object::vector_t output_list;

  // Loop over all detections and remove all detections that have most
  // likely class-names that have been specified in the ignore_classes
  // list.
  VITAL_FOREACH( auto det, det_list)
  {
    auto dot = det->type();
    if ( ! dot )
    {
      // this detection is not classified
      continue;
    }

    std::string max_class_name;
    double max_score;

    try
    {
      dot->get_most_likely( max_class_name, max_score );

      if( std::find(ignore_classes.begin(), ignore_classes.end(), max_class_name) == ignore_classes.end() )
      {
        // This class names is not in the ignore list
        det->set_confidence( max_score );
        output_list.push_back( det );
      }
    }
    catch ( ... ) { }
  } // end foreach detection

  // do a second pass over the selected outputs
  for ( size_t i = 0; i < output_list.size(); ++i)
  {
    auto det = output_list[i];
    auto dot_i = det->type();
    auto bbox_i = det->bounding_box();

    double ai = bbox_i.area();

    // Scan remaining detections to collect max scores
    for ( size_t j = i + 1; j < output_list.size(); ++j )
    {
      auto obj_j = output_list[j];
      auto dot_j = obj_j->type();
      auto bbox_j = obj_j->bounding_box();

      // Set class scores for this detection to the maximum of scores
      // in [i] and [j] on a name by name basis.
      auto all_name_list = kwiver::vital::detected_object_type::all_class_names();
      VITAL_FOREACH( auto name, all_name_list)
      {
        // If [j] has the class then copy score to [i] if [i] does not have class or score is lower.
        if ( dot_j->has_class_name( name )
             && ( ! dot_i->has_class_name( name ) || ( dot_i->score( name ) < dot_j->score( name ) ) ) )
        {
          dot_i->set_score( name, dot_j->score( name ) );
        }
      } // end foreach over class names

      // Check overlap between [i] and [j].
      // Delete [j] if needed
      auto inter = intersection( bbox_i, bbox_j );
      double aj = bbox_j.area();
      double interS = inter.area();
      double t = interS / ( ai + aj - interS );

      // If sufficient overlap, then delete this detection [j]
      if ( t >= 0.3 )
      {
        auto it = output_list.begin() + j;
        output_list.erase( it );
      }
    }
  } // end for i over selected detections

  return std::make_shared< vital::detected_object_set > (output_list );
}


// ==================================================================
class draw_detected_object_boxes_process::priv
{
public:
  priv()
    : m_count( 0 )
  {
    m_draw_overlap_max = true;
  }

  ~priv()
  { }


  mutable size_t m_count;

  // Configuration values
  float m_threshold;
  std::vector< std::string > m_ignore_classes;
  bool m_do_alpha;

  struct Bound_Box_Params
  {
    float thickness;
    ColorVector color;
  } m_default_params;

  // box attributes per object type
  std::map< std::string, Bound_Box_Params > m_custum_colors;
  float m_text_scale;
  float m_text_thickness;
  bool m_clip_box_to_image;
  bool m_draw_overlap_max;
  bool m_draw_text;
  bool m_draw_other_classes;
  
  kwiver::vital::wall_timer m_timer;


  // ------------------------------------------------------------------
  /**
   * @brief Draw a box on an image.
   *
   * This method draws a box on an image for the bounding box from a
   * detected object.
   *
   * @param[in,out] image Input image updated with drawn box
   * @param[in] dos detected object with bounding box
   * @param tmpT
   * @param label Text label to use for box
   * @param prob Probability value to add to label text
   * @param just_text Set to true if only draw text, not the bounding box. This is used
   *        when there are multiple labels for the same detection.
   * @param offset How much to offset text fill box from text baseline. This is used to
   *        offset labels when there are more than one label for a detection.
   */
  void draw_box( cv::Mat&                     image,
                 const vital::detected_object_sptr  dos,
                 double                       tmpT,
                 std::string                  label,
                 double                       prob,
                 bool                         just_text = false,
                 int                          offset = multi_label_offset ) const
  {
    cv::Mat overlay;

    image.copyTo( overlay );
    auto bbox = dos->bounding_box();

    if ( m_clip_box_to_image )
    {
      cv::Size s = image.size();
      vital::bounding_box_d img( vital::vector_2d( 0, 0 ),
                                 vital::vector_2d( s.width, s.height ) );
      bbox = intersection( img, bbox );
    }

    cv::Rect r( bbox.upper_left()[0], bbox.upper_left()[1], bbox.width(), bbox.height() );
    std::string p = std::to_string( prob );
    std::string txt = label + " " + p;
    prob =  ( m_do_alpha ) ? ( ( prob - tmpT ) / ( 1 - tmpT ) ) : 1.0;
    Bound_Box_Params const* bbp = &m_default_params;
    std::map< std::string, Bound_Box_Params >::const_iterator iter = m_custum_colors.find( label );

    if ( iter != m_custum_colors.end() )
    {
      bbp = &( iter->second );
    }

    if ( ! just_text )
    {
      cv::Scalar color( bbp->color[0], bbp->color[1], bbp->color[2] );
      cv::rectangle( overlay, r, color, bbp->thickness );
    }

    if ( m_draw_text )
    {
      int fontface = cv::FONT_HERSHEY_SIMPLEX;
      double scale = m_text_scale;
      int thickness = m_text_thickness;
      int baseline = 0;
      cv::Point pt( r.tl() + cv::Point( 0, offset ) );

      cv::Size text = cv::getTextSize( txt, fontface, scale, thickness, &baseline );
      cv::rectangle( overlay, pt + cv::Point( 0, baseline ), pt +
                     cv::Point( text.width, -text.height ), cv::Scalar( 0, 0, 0 ), CV_FILLED );

      cv::putText( overlay, txt, pt, fontface, scale, cv::Scalar( 255, 255, 255 ), thickness, 8 );
    }

    cv::addWeighted( overlay, prob, image, 1 - prob, 0, image );
  } // draw_box


  // ------------------------------------------------------------------
  /**
   * @brief Draw detected object on image.
   *
   * @param image_data The image to draw on.
   * @param input_set List of detections to draw.
   *
   * @return Updated image.
   */
  vital::image_container_sptr
  draw_on_image( vital::image_container_sptr      image_data,
                 vital::detected_object_set_sptr  in_set ) const
  {
    if ( in_set == NULL )
    {
      throw kwiver::vital::invalid_value( "Detected object set pointer is NULL" );
    }

    if ( image_data == NULL )
    {
      throw kwiver::vital::invalid_value( "Input image pointer is NULL" );
    }

    cv::Mat image = arrows::ocv::image_container::vital_to_ocv( image_data->get_image() ).clone();
    cv::Mat overlay;

    vital::detected_object_set_sptr  input_set = in_set;
    double tmpT = ( this->m_threshold - ( ( this->m_threshold >= 0.05 ) ? 0.05 : 0 ) );

    if ( m_draw_overlap_max )
    {
      input_set = NMS_COMBINER( in_set, this->m_ignore_classes );

      auto det_list = input_set->select();
      VITAL_FOREACH( auto det, det_list )
      {
        auto dot_i = det->type();
        if ( m_draw_other_classes )
        {
          auto name_list = dot_i->class_names( this->m_threshold );
          if ( name_list.size() > 0 ) // if list is empty
          {
            continue;
          }

          auto bbox = det->bounding_box();
          draw_box( image, det, tmpT, name_list[0], dot_i->score( name_list[0] ) );
          int tmp_off = 2 * multi_label_offset;

          for (size_t i = 1; i < name_list.size(); ++i)
          {
            draw_box( image, det, tmpT, name_list[i], dot_i->score( name_list[i] ) );
            tmp_off += multi_label_offset;
          }
        }
        else
        {
          std::string max_label;
          double max_score;

          try
          {
            dot_i->get_most_likely( max_label, max_score );
            if ( max_score <= this->m_threshold )
            {
              continue;
            }

            draw_box( image, det, tmpT, max_label, max_score );
          }
          catch (... ) { }
        }
      }
    } // end draw overlap max
    else
    {
      auto name_list = kwiver::vital::detected_object_type::all_class_names();
      VITAL_FOREACH( auto name, name_list )
      {
        // This class names is in the ignore list, go to next name
        if( std::find(m_ignore_classes.begin(), m_ignore_classes.end(), name) != m_ignore_classes.end() )
        {
          continue;
        }

        // Select all class-names not less than threshold
        auto det_set = input_set->select( name, this->m_threshold );

        // Check to see if there are any items to process
        if (det_set.size() == 0 )
        {
          continue;
        }

        double tmpT = ( this->m_threshold - ( ( this->m_threshold >= 0.05 ) ? 0.05 : 0 ) );

        // Draw in reverse to highest score is on top
        for ( size_t i = det_set.size() - 1; i >= 0; --i )
        {
          auto det = det_set[i]; // low score first
          draw_box( image, det, tmpT, name, det->type()->score( name ) );
        }
      }
    }

    return vital::image_container_sptr( new arrows::ocv::image_container( image ) );
  }   // draw_on_image

}; // end priv class


// ==================================================================
draw_detected_object_boxes_process
::draw_detected_object_boxes_process( vital::config_block_sptr const& config )
  : process( config ),
  d( new draw_detected_object_boxes_process::priv )
{
  // Attach our logger name to process logger
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}


draw_detected_object_boxes_process
  ::~draw_detected_object_boxes_process()
{
}


// ------------------------------------------------------------------
void
draw_detected_object_boxes_process::_configure()
{
  d->m_threshold          = config_value_using_trait( threshold );
  d->m_clip_box_to_image  = config_value_using_trait( clip_box_to_image );
  d->m_draw_text          = config_value_using_trait( draw_text );
  d->m_draw_overlap_max   = config_value_using_trait( merge_overlapping_classes );
  d->m_draw_other_classes = config_value_using_trait( draw_other_classes );

  std::string parsed;
  std::string list = config_value_using_trait( ignore_file );

  {
    std::stringstream ss( list );

    while ( std::getline( ss, parsed, ';' ) )
    {
      if ( ! parsed.empty() )
      {
        d->m_ignore_classes.push_back( parsed );
      }
    }
  }

  d->m_do_alpha = config_value_using_trait( alpha_blend_prob );
  d->m_default_params.thickness = config_value_using_trait( default_line_thickness );

  { // parse defaults
    std::stringstream lss( config_value_using_trait( default_color ) );
    lss >> d->m_default_params.color[0] >> d->m_default_params.color[1] >> d->m_default_params.color[2];
  }

  std::string custom = config_value_using_trait( custom_class_color );

  d->m_text_scale = config_value_using_trait( text_scale );
  d->m_text_thickness = config_value_using_trait( text_thickness );

  {
    std::stringstream ss( custom );

    while ( std::getline( ss, parsed, ';' ) )
    {
      if ( ! parsed.empty() )
      {
        std::stringstream sub( parsed );
        std::string cl, t, co;
        std::getline( sub, cl, '/' );
        std::getline( sub, t, '/' );
        std::getline( sub, co, '/' );
        std::stringstream css( co );

        draw_detected_object_boxes_process::priv::Bound_Box_Params bp;
        bp.thickness = std::stof( t );
        css >> bp.color[0] >> bp.color[1] >> bp.color[2];
        d->m_custum_colors[cl] = bp;
      }
    }
  }
} // draw_detected_object_boxes_process::_configure


// ------------------------------------------------------------------
void
draw_detected_object_boxes_process::_step()
{
  d->m_timer.start();
  auto img = grab_from_port_using_trait( image );
  auto detections = grab_from_port_using_trait( detected_object_set );

  auto result = d->draw_on_image( img, detections );

  push_to_port_using_trait( image, result );
  double elapsed_time = d->m_timer.elapsed();
  LOG_DEBUG( logger(), "Total processing time: " << elapsed_time );
  std::cout << "HERE!!!" << std::endl;
}


// ------------------------------------------------------------------
void
draw_detected_object_boxes_process::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( detected_object_set, required );
  declare_input_port_using_trait( image, required );

  // -- output --
  declare_output_port_using_trait( image, optional );
}


// ------------------------------------------------------------------
void
draw_detected_object_boxes_process::make_config()
{
  declare_config_using_trait( threshold );
  declare_config_using_trait( ignore_file );
  declare_config_using_trait( file_string );
  declare_config_using_trait( alpha_blend_prob );
  declare_config_using_trait( default_line_thickness );
  declare_config_using_trait( default_color );
  declare_config_using_trait( custom_class_color );
  declare_config_using_trait( text_scale );
  declare_config_using_trait( text_thickness );
  declare_config_using_trait( clip_box_to_image );
  declare_config_using_trait( draw_text );
  declare_config_using_trait( merge_overlapping_classes );
  declare_config_using_trait( draw_other_classes );
}

} //end namespace
