/*ckwg +29
 * Copyright 2016-2017 by Kitware, Inc.
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

#include "detected_object_set_output_kpf.h"

#include <vital/util/tokenize.h>

#include <arrows/kpf/yaml/kpf_canonical_io_adapter.h>
#include <arrows/kpf/yaml/kpf_yaml_writer.h>

#include <arrows/kpf/vital_kpf_adapters.h>

#include <memory>
#include <vector>
#include <fstream>
#include <time.h>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#if ( __GNUC__ == 4 && __GNUC_MINOR__ < 5 && !defined(__clang__) )
  #include <cstdatomic>
#else
  #include <atomic>
#endif


namespace kwiver {
namespace arrows {
namespace kpf {

// ------------------------------------------------------------------
class detected_object_set_output_kpf::priv
{
public:
  priv( detected_object_set_output_kpf* parent)
    : m_parent( parent )
    , m_logger( kwiver::vital::get_logger( "detected_object_set_output_kpf" ) )
    , m_frame_number( 1 )
  {}

  ~priv() {}

  void read_all();

  detected_object_set_output_kpf* m_parent;
  kwiver::vital::logger_handle_t m_logger;
  int m_frame_number;
};


// ==================================================================
detected_object_set_output_kpf::
detected_object_set_output_kpf()
  : d( new detected_object_set_output_kpf::priv( this ) )
{
}


detected_object_set_output_kpf::
~detected_object_set_output_kpf()
{
  
}


// ------------------------------------------------------------------
void
detected_object_set_output_kpf::
set_configuration( vital::config_block_sptr config_in )
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( config_in );
}


// ------------------------------------------------------------------
vital::config_block_sptr
detected_object_set_output_kpf::
get_configuration() const
{
  // get base config from base class
  kwiver::vital::config_block_sptr config = algorithm::get_configuration();

  return config;
}


// ------------------------------------------------------------------
bool
detected_object_set_output_kpf::
check_configuration( vital::config_block_sptr config ) const
{
  return true;
}

// ------------------------------------------------------------------
void
detected_object_set_output_kpf::
write_set( const kwiver::vital::detected_object_set_sptr set, std::string const& image_name )
{
  KPF::record_yaml_writer w(stream());
  size_t line_count = 0;
  vital_box_adapter_t box_adapter;
  // Is this needed? vital_poly_adapter_t poly_adapter;

  // process all detections
  auto ie =  set->cend();
  for ( auto det = set->cbegin(); det != ie; ++det )
  {
    const kwiver::vital::bounding_box_d bbox( (*det)->bounding_box() );
    // N/A? double ilx = ( bbox.min_x() + bbox.max_x() ) / 2.0;
    // N/A? double ily = ( bbox.min_y() + bbox.max_y() ) / 2.0;

    static std::atomic<unsigned> id_counter( 0 );
    const unsigned id = id_counter++;

    std::ostringstream oss;
    oss << "Record " << line_count++;
    w
      << KPF::writer< KPFC::meta_t >(oss.str())
      << KPF::record_yaml_writer::endl;
    w
      << KPF::writer< KPFC::kv_t >("detector_name", (*det)->detector_name())
      << KPF::writer< KPFC::id_t >(id, KPFC::id_t::DETECTION_ID)
      << KPF::writer< KPFC::timestamp_t >(d->m_frame_number - 1, KPFC::timestamp_t::FRAME_NUMBER)
      << KPF::writer< KPFC::conf_t>((*det)->confidence(), DETECTOR_DOMAIN)
      << KPF::writer< KPFC::bbox_t >(box_adapter((*det)->bounding_box()), KPFC::bbox_t::IMAGE_COORDS);
    //for (auto t : *(*det)->type())
    //  w << KPF::writer< KPFC::kv_t >(*t.first, std::to_string(t.second));
    w << KPF::record_yaml_writer::endl;


    // I do not think the vital detection has poly data....
    //<< KPF::writer< KPFC::poly_t>(poly_adapter(det), KPFC::poly_t::IMAGE_COORDS)
    /*
    This is how kw18 writes out data, Delete when we are happy with the above code
    stream() << id                  // 1: track id
             << " 1 "               // 2: track length
             << d->m_frame_number-1 // 3: frame number / set number
             << " 0 "               // 4: tracking plane x
             << " 0 "               // 5: tracking plane y
             << "0 "                // 6: velocity x
             << "0 "                // 7: velocity y
             << ilx << " "          // 8: image location x
             << ily << " "          // 9: image location y
             << bbox.min_x() << " " // 10: TL-x
             << bbox.min_y() << " " // 11: TL-y
             << bbox.max_x() << " " // 12: BR-x
             << bbox.max_y() << " " // 13: BR-y
             << bbox.area() << " "  // 14: area
             << "0 "                // 15: world-loc x
             << "0 "                // 16: world-loc y
             << "0 "                // 17: world-loc z
             << "-1 "                // 18: timestamp
             << (*det)->confidence()   // 19: confidence
             << std::endl;
    */
  } // end foreach

  // Put each set on a new frame
  ++d->m_frame_number;
}

} } } // end namespace
