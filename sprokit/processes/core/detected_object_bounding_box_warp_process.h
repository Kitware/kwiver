/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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
 * \brief Interface to detected object updater process.
 */

#ifndef _DETECTED_OBJECT_BOUNDING_BOX_WARP_PROCESS_H
#define _DETECTED_OBJECT_BOUNDING_BOX_WARP_PROCESS_H

#include <sprokit/pipeline/process.h>
#include "kwiver_processes_export.h"
#include <vital/config/config_block.h>

namespace kwiver {

// ----------------------------------------------------------------
/**
 * \class detected_object_bounding_box_warp_process
 *
 * \brief Warp bounding boxes of a detected_object_set with a homography
 *
 * \iports
 * \iport{detected_object_set}
 * \iport{homography}
 *
 * \oports
 * \oport{detected_object_set}
 */

class KWIVER_PROCESSES_NO_EXPORT detected_object_bounding_box_warp_process
  : public sprokit::process
{
public:
  detected_object_bounding_box_warp_process( kwiver::vital::config_block_sptr const& config );
  virtual ~detected_object_bounding_box_warp_process();

protected:
  virtual void _step();

private:
  void make_ports();
}; // end class detected_object_bounding_box_warp_process

} //namespace kwiver

#endif //_DETECTED_OBJECT_BOUNDING_BOX_WARP_PROCESS_H
