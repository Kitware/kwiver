/*ckwg +29
 * Copyright 2017-2018 by Kitware, Inc.
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

#ifndef _KWIVER_COMPUTE_TRACK_DESCRIPTORS_PROCESS_H_
#define _KWIVER_COMPUTE_TRACK_DESCRIPTORS_PROCESS_H_

#include "kwiver_processes_export.h"

#include <sprokit/pipeline/process.h>

#include <vital/types/track_descriptor_set.h>

#include <memory>

namespace kwiver
{

// -----------------------------------------------------------------------------
/**
 * \class compute_track_descriptors_process
 *
 * \brief Computes track descriptors along object tracks or object detections.
 *
 * \iports
 * \iport{timestamp}
 * \iport{image}
 * \iport{tracks}
 * \iport{detections}
 *
 * \oports
 * \oport{track_descriptor_set}
 */
class KWIVER_PROCESSES_NO_EXPORT compute_track_descriptors_process
  : public sprokit::process
{
public:
  PLUGIN_INFO( "compute_track_descriptors",
               "Compute track descriptors on the input tracks or detections." )

  compute_track_descriptors_process( vital::config_block_sptr const& config );
  virtual ~compute_track_descriptors_process();

protected:
  virtual void _configure();
  virtual void _step();

private:
  void make_ports();
  void make_config();

  void push_outputs( vital::track_descriptor_set_sptr& to_output );

  class priv;
  const std::unique_ptr<priv> d;
}; // end class compute_track_descriptors_process


} // end namespace
#endif /* _KWIVER_COMPUTE_TRACK_DESCRIPTORS_PROCESS_H_ */
