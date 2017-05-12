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

#include "timing_object_csv_process.h"

#include <vital/vital_types.h>
#include <vital/types/image_container.h>
#include <vital/types/image.h>
#include <vital/types/timestamp.h>
#include <vital/exceptions.h>
#include <vital/util/string.h>
#include <vital/util/wall_timer.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>
#include <sprokit/pipeline/datum.h>

#include <kwiversys/SystemTools.hxx>

#include <vector>
#include <stdint.h>
#include <fstream>


namespace kwiver {

// (config-key, value-type, default-value, description )
create_config_trait( output_csv_file, std::string, "out.csv",
                     "The output csv file" );

//----------------------------------------------------------------
// Private implementation class
class timing_object_csv_process::priv
{
public:
  priv();
  ~priv();

  // Configuration values
  std::string m_file_name;

  // Number for current image.
  kwiver::vital::timestamp m_timestamp;

  std::ofstream m_csv_file;

}; // end priv class


// ================================================================

timing_object_csv_process
::timing_object_csv_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new timing_object_csv_process::priv )
{
  // Attach our logger name to process logger
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}


timing_object_csv_process
::~timing_object_csv_process()
{
}


// ----------------------------------------------------------------
void timing_object_csv_process
::_configure()
{
  // Get process config entries
  d->m_file_name = config_value_using_trait( output_csv_file );
  d->m_csv_file.open(d->m_file_name);
  if(!d->m_csv_file)
  {
    throw sprokit::invalid_configuration_exception( name(), "Could not open file: " + d->m_file_name );
  }
  d->m_csv_file << "# frame-number,frame-time,image-file-name,number-of-detections,process-time\n";

  // Get algo config entries
  kwiver::vital::config_block_sptr algo_config = get_config(); // config for process
}


// ----------------------------------------------------------------
void timing_object_csv_process
::_step()
{
  kwiver::vital::timestamp frame_time =
                          grab_from_port_using_trait( timestamp );
  vital::detected_object_set_sptr detects =
                grab_from_port_using_trait( detected_object_set );
  double m_detect_timer =
                     grab_from_port_using_trait( detection_time );
  std::string fname;
  if (has_input_port_edge_using_trait( image_file_name ) )
  {
    fname = grab_input_using_trait( image_file_name );
  }
  d->m_csv_file << frame_time.get_frame() << ","
                << frame_time.get_time_seconds() << ","
                << fname << ","
                << detects->size() << ","
                << m_detect_timer << std::endl;
}


// ----------------------------------------------------------------
void timing_object_csv_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  declare_input_port_using_trait( timestamp, required);
  declare_input_port_using_trait( detected_object_set, required );
  declare_input_port_using_trait( detection_time, required );
  declare_input_port_using_trait( image_file_name, optional );
}


// ----------------------------------------------------------------
void timing_object_csv_process
::make_config()
{
  declare_config_using_trait( output_csv_file );
}


// ================================================================
timing_object_csv_process::priv
::priv()
{
}


timing_object_csv_process::priv
::~priv()
{
}

} // end namespace
