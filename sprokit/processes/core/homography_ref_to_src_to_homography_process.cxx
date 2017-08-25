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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS [yas] elisp error!AS IS''
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

#include "homography_ref_to_src_to_homography_process.h"

#include <vital/vital_types.h>
#include <vital/types/timestamp.h>
#include <vital/types//homography_f2f.h>
#include <vital/types/timestamp_config.h>
#include <vital/types/image_container.h>
#include <vital/util/wall_timer.h>
#include <arrows/ocv/image_container.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace kwiver {

//----------------------------------------------------------------
// Private implementation class
class homography_ref_to_src_to_homography_process::priv
{
public:
  priv();
  ~priv();

  // Configuration values
  kwiver::vital::wall_timer m_timer;

}; // end priv class

// ================================================================

homography_ref_to_src_to_homography_process
::homography_ref_to_src_to_homography_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new homography_ref_to_src_to_homography_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}


homography_ref_to_src_to_homography_process
::~homography_ref_to_src_to_homography_process()
{
}


// ----------------------------------------------------------------
void homography_ref_to_src_to_homography_process
::_configure()
{
}


// ----------------------------------------------------------------
void
homography_ref_to_src_to_homography_process
::_step()
{
  d->m_timer.start();
  
  // input homography
  kwiver::vital::homography_f2f_sptr homog_f2f = grab_from_port_using_trait( homography_src_to_ref );
  
  kwiver::vital::homography_sptr homog = homog_f2f->homography();
  
  push_to_port_using_trait( homography, homog );
  
  d->m_timer.stop();
  double elapsed_time = d->m_timer.elapsed();
  LOG_DEBUG( logger(), "Total processing time: " << elapsed_time << " seconds");
}


// ----------------------------------------------------------------
void homography_ref_to_src_to_homography_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( homography_src_to_ref, required );

  // -- output --
  declare_output_port_using_trait( homography, required );
}


// ----------------------------------------------------------------
void homography_ref_to_src_to_homography_process
::make_config()
{
}


// ================================================================
homography_ref_to_src_to_homography_process::priv
::priv()
{
}


homography_ref_to_src_to_homography_process::priv
::~priv()
{
}

} // end namespace
