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

#include "draw_detected_object_boxes_process.h"

#include <vital/vital_types.h>

#include <arrows/algorithms/ocv/image_container.h>
#include <arrows/processes/kwiver_type_traits.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <arrows/algorithms/ocv/image_container.h>
#include <arrows/algorithms/ocv/image_container.h>

#include <Eigen/Core>

#include <sstream>
#include <iostream>

namespace kwiver {

typedef  Eigen::Matrix< unsigned int, 3, 1 > ColorVector;

create_config_trait( threshold, float, "-1", "min probablity for output (float)" );
create_config_trait( alpha_blend_prob, bool, "true", "If true, those who are less likely will be more transparent." );
create_config_trait( default_line_thickness, float, "1", "The default line thickness for a class" );
create_config_trait( default_color, std::string, "255 0 0", "The default color for a class (BGR)" );
create_config_trait( custom_class_color,
                     std::string,
                     "",
                     "List of class/thickness/color seperated by semi-colon. For example: person/3/255 0 0;car/2/0 255 0" );
create_config_trait( ignore_file, std::string, "__background__", "List of classes to ingore, seperated by semi-colon." );
create_config_trait( text_scale, float, "0.4", "the scale for the text label" );
create_config_trait( text_thickness, float, "1.0", "the thickness for text" );
create_config_trait( file_string, std::string, "", "If not empty, use this as a formated string to write output (i.e. out_%5d.png)" );
create_config_trait( clip_box_to_image, bool, "false", "make sure the bounding box is only in the image");
create_config_trait( draw_text, bool, "true", "Draw the text");
create_config_trait( merge_overlapping_classes, bool, "true", "Combine overlapping classes");
create_config_trait( draw_other_classes, bool, "false", "Print all combined overlap");

vital::detected_object_set_sptr NMS_COMBINER( vital::detected_object_set_sptr input_set, std::vector<std::string> const& ignore_classes)
{
  vital::detected_object_set::iterator iter = input_set->get_iterator();
  for ( ; !iter.is_end(); ++iter )
  {
    vital::detected_object_sptr dos = iter.get_object();
    std::string max_label;
    double max_score = (dos->get_classifications() != NULL)?dos->get_classifications()->get_max_score(max_label):vital::object_type::INVALID_SCORE;
    for ( size_t i = 0; i < ignore_classes.size(); ++i )
      {
        if (  ignore_classes[i] == max_label )
        {
           max_score = vital::object_type::INVALID_SCORE;
        }
      }
    dos->set_confidence(max_score);
  }
  vital::detected_object_set::iterator class_iterator = input_set->get_iterator(true);
  std::vector<vital::detected_object_sptr> tmp;

  for ( size_t i = 0; i < class_iterator.size(); ++i )
  {
      vital::detected_object_sptr obj_i = class_iterator[i];
      vital::object_type_sptr class_i = obj_i->get_classifications();
      vital::detected_object::bounding_box bbox_i = class_iterator[i]->get_bounding_box();
      if(obj_i->get_confidence() == vital::object_type::INVALID_SCORE) continue;

      double area = bbox_i.area();
      tmp.push_back(obj_i);
      for ( size_t j = i+1; j < class_iterator.size(); ++j )
      {
        vital::detected_object_sptr obj_j = class_iterator[j];
        vital::object_type_sptr class_j = obj_j->get_classifications();
        vital::detected_object::bounding_box bbox_j = class_iterator[j]->get_bounding_box();
        if(obj_i->get_confidence() == vital::object_type::INVALID_SCORE) continue;
        vital::object_labels::iterator label_iter = input_set->get_labels();
        for(;!label_iter.is_end(); ++label_iter)
        {
            if( class_j->get_score(label_iter.get_key()) != vital::object_type::INVALID_SCORE &&  class_i->get_score(label_iter.get_key()) < class_j->get_score(label_iter.get_key()) )
            {
               class_i->set_score(label_iter.get_key(), class_j->get_score(label_iter.get_key()));
            }
        }
        vital::detected_object::bounding_box inter = bbox_i.intersection(bbox_j);
        double aj = bbox_j.area();
        double interS = inter.area();
        double t = interS / (area + aj - interS);
        if (t >= 0.3)
        {
          obj_j->set_confidence(vital::object_type::INVALID_SCORE);
        }
      }
  }
  return vital::detected_object_set_sptr(new vital::detected_object_set(tmp, input_set->get_object_labels()));
}

class draw_detected_object_boxes_process::priv
{
public:
  priv()
    : m_count( 0 )
  { m_draw_overlap_max = true; }

  ~priv()
  { }


  mutable size_t m_count;
  std::string m_formated_string;

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

  void draw_box(vital::image_container_sptr image_data, cv::Mat & image, vital::detected_object_sptr dos, double tmpT, std::string label, double prob) const
  {
    cv::Mat overlay;
    image.copyTo( overlay );
    vital::detected_object::bounding_box bbox = dos->get_bounding_box();
    if ( m_clip_box_to_image )
    {
       vital::detected_object::bounding_box img( vital::vector_2d( 0, 0 ),
                                                    vital::vector_2d( image_data->width(),
                                                                      image_data->height() ) );
       bbox = img.intersection( bbox );
    }

    cv::Rect r( bbox.upper_left()[0], bbox.upper_left()[1], bbox.width(), bbox.height() );
    std::string p = std::to_string( prob );
    std::string txt = label + " " + p;
    prob =  ( m_do_alpha ) ? (( prob - tmpT ) / ( 1 - tmpT )) : 1.0;
    Bound_Box_Params const* bbp = &m_default_params;
    std::map< std::string, Bound_Box_Params >::const_iterator iter = m_custum_colors.find( label );

    if ( iter != m_custum_colors.end() )
    {
      bbp = &( iter->second );
    }

    cv::Scalar color( bbp->color[0], bbp->color[1], bbp->color[2] );
    cv::rectangle( overlay, r, color, bbp->thickness );

    if(m_draw_text)
    {
          int fontface = cv::FONT_HERSHEY_SIMPLEX;
          double scale = m_text_scale;
          int thickness = m_text_thickness;
          int baseline = 0;
          cv::Point pt( r.tl() + cv::Point( 0, 15 ) );

          cv::Size text = cv::getTextSize( txt, fontface, scale, thickness, &baseline );
          cv::rectangle( overlay, pt + cv::Point( 0, baseline ), pt +
                         cv::Point( text.width, -text.height ), cv::Scalar( 0, 0, 0 ), CV_FILLED );

          cv::putText( overlay, txt, pt, fontface, scale, cv::Scalar( 255, 255, 255 ), thickness, 8 );
     }

     cv::addWeighted( overlay, prob, image, 1 - prob, 0, image );
  }
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
    if ( in_set == NULL ) { return image_data; }
    if ( image_data == NULL ) { return NULL; } // Maybe throw?

    cv::Mat image = arrows::ocv::image_container::vital_to_ocv( image_data->get_image() ).clone();
    cv::Mat overlay;
    vital::object_labels::iterator label_iter = in_set->get_labels();

    vital::detected_object_set_sptr  input_set = in_set;

    if(m_draw_overlap_max)
    {
      input_set = NMS_COMBINER(in_set, this->m_ignore_classes);
      vital::detected_object_set::iterator class_iterator = input_set->get_iterator();
      for ( size_t i = 0; i < class_iterator.size(); ++i )
      {
        vital::object_type_sptr ots = class_iterator[i]->get_classifications();
        /*if(m_draw_other_classes)
        {
          vital::object_type::iterator iter = ots->get_iterator(true, this->m_threshold);
          if(iter.is_end()) continue;
          draw_box(image_data, image, class_iterator[i], tmpT, iter.get_label(), iter.get_score());
          ++iter;
          int tmp_off = 30
          for(;!iter.is_end() && m_draw_text;++iter)
          {
             int fontface = cv::FONT_HERSHEY_SIMPLEX;
             double scale = m_text_scale;
             int thickness = m_text_thickness;
             int baseline = 0;
             cv::Point pt( r.tl() + cv::Point( 0, tmp_off ) );

             cv::Size text = cv::getTextSize( txt, fontface, scale, thickness, &baseline );
             cv::rectangle( overlay, pt + cv::Point( 0, baseline ), pt +
                           cv::Point( text.width, -text.height ), cv::Scalar( 0, 0, 0 ), CV_FILLED );

             cv::putText( overlay, txt, pt, fontface, scale, cv::Scalar( 255, 255, 255 ), thickness, 8 );
          }

        }
        else*/
        {
          std::string max_label;
          double d = ots->get_max_score(max_label);
          if(d <= this->m_threshold) continue;
          double tmpT = ( this->m_threshold - ( ( this->m_threshold >= 0.05 ) ? 0.05 : 0 ) );
          draw_box(image_data, image, class_iterator[i], tmpT, max_label, d);
        }
      }
    }
    else
    {
      for ( ; ! label_iter.is_end(); ++label_iter )
      {
        bool keep_going = true;
        for ( size_t i = 0; i < this->m_ignore_classes.size(); ++i )
        {
          if ( this->m_ignore_classes[i] == label_iter.get_label() )
          {
            keep_going = false;
            break;
          }
        }

        if ( ! keep_going ) { continue; }

        vital::detected_object_set::iterator class_iterator =
          input_set->get_iterator( label_iter.get_key(), true, this->m_threshold );

        // Check to see if there are any items to process
        if ( class_iterator.size() == 0 ) { continue; }

        double tmpT = ( this->m_threshold - ( ( this->m_threshold >= 0.05 ) ? 0.05 : 0 ) );

        for ( size_t i = class_iterator.size() - 1; i < class_iterator.size() && i >= 0; --i )
        {
          vital::detected_object_sptr dos = class_iterator[i];         //Low score first
          draw_box(image_data, image, dos, tmpT, label_iter.get_label(), dos->get_classifications()->get_score( label_iter.get_key() ));
        }   
      }
    }

    if ( ! m_formated_string.empty() )
    {
      char buffer[1024];

      sprintf( buffer, m_formated_string.c_str(), m_count );
      ++m_count;
      cv::imwrite( buffer, image );
    }
    return vital::image_container_sptr(new arrows::ocv::image_container(image));
  } // draw_on_image

}; // end priv class


draw_detected_object_boxes_process
  ::draw_detected_object_boxes_process( vital::config_block_sptr const& config )
  : process( config ),
  d( new draw_detected_object_boxes_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach
  make_ports();
  make_config();
}


draw_detected_object_boxes_process
  ::~draw_detected_object_boxes_process()
{
}


void
draw_detected_object_boxes_process::_configure()
{
  d->m_threshold = config_value_using_trait( threshold );
  d->m_formated_string = config_value_using_trait( file_string );
  d->m_clip_box_to_image = config_value_using_trait( clip_box_to_image );
  d->m_draw_text = config_value_using_trait(draw_text);
  d->m_draw_overlap_max = config_value_using_trait(merge_overlapping_classes);
  d->m_draw_other_classes = config_value_using_trait(draw_other_classes);

  std::string parsed, list = config_value_using_trait(ignore_file);

  {
    std::stringstream ss(list);

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


void
draw_detected_object_boxes_process::_step()
{
  vital::image_container_sptr img = grab_from_port_using_trait( image );
  vital::detected_object_set_sptr detections = grab_from_port_using_trait( detected_object_set );
  vital::image_container_sptr result = d->draw_on_image( img, detections );

  push_to_port_using_trait( image, result );
}


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

  //output
  declare_output_port_using_trait( image, optional );
}


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
