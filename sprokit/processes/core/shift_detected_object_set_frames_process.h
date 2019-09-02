/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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

#ifndef SPROKIT_PROCESSES_SHIFT_DETECTED_OBJECT_SET_FRAMES_PROCESS_H
#define SPROKIT_PROCESSES_SHIFT_DETECTED_OBJECT_SET_FRAMES_PROCESS_H

#include <sprokit/pipeline/process.h>

#include "kwiver_processes_export.h"

/**
 * \file shift_process.h
 *
 * \brief Declaration of the detected object set frame shift process
 */

namespace kwiver
{

/**
 * \class shift_detected_object_set_frames_process
 *
 * \brief Shifts input stream of detected object sets
 *
 * \process Either
 *
 * \iports
 *
 * \iports{any} The input detected object set
 *
 * \oports
 *
 * \oport{any} The ith - offset detected object set from the input stream.
 *             Any items outside the input stream bounds are provided as empty
 *             detected object sets
 * *
 * \reqs
 *
 * \req The \port{any} input must be connected.
 * \req The \port{any} output must be connected.
 *
 * \ingroup examples
 */
class KWIVER_PROCESSES_NO_EXPORT shift_detected_object_set_frames_process
  : public sprokit::process
{
  public:
    PLUGIN_INFO( "shift_detected_object_set",
                 "Shift an input stream of detected objects "
		 "by a certain number of frames")
    /**
     * \brief Constructor.
     *
     * \param config The configuration for the process.
     */
    shift_detected_object_set_frames_process
      (kwiver::vital::config_block_sptr const& config);
    /**
     * \brief Destructor.
     */
    ~shift_detected_object_set_frames_process();
  protected:
    /**
     * \brief Configure the process.
     */
    void _configure();

    /**
     * \brief Step the process.
     */
    void _step();
  private:
    void make_ports();
    void make_config();

    class priv;
    std::unique_ptr<priv> d;
};

}

#endif // SPROKIT_PROCESSES_SHIFT_DETECTED_OBJECT_SET_FRAMES_PROCESS_H
