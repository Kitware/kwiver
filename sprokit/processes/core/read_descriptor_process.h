/*ckwg +29
 * Copyright 2015-2017 by Kitware, Inc.
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
 * \brief Image display process interface.
 */

#ifndef _KWIVER_READ_DESCRIPTOR_PROCESS_H
#define _KWIVER_READ_DESCRIPTOR_PROCESS_H

#include <sprokit/pipeline/process.h>
#include "kwiver_processes_export.h"

#include <memory>

namespace kwiver
{

// -------------------------------------------------------------------------------
/**
 * @brief Display images
 *
 */
class KWIVER_PROCESSES_NO_EXPORT read_descriptor_process
  : public sprokit::process
{
public:
  PLUGIN_INFO( "read_d_vector",
               "Read vector of doubles," )

  // -- CONSTRUCTORS --
  read_descriptor_process( kwiver::vital::config_block_sptr const& config );
  virtual ~read_descriptor_process();

protected:
  virtual void _configure();
  virtual void _step();

private:
  void make_ports();
  void make_config();


  class priv;
  const std::unique_ptr<priv> d;

}; // end class read_descriptor_process

} // end namespace

#endif /* _KWIVER_READ_DESCRIPTOR_PROCESS_H */
