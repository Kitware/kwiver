/*ckwg +29
 * Copyright 2012-2017 by Kitware, Inc.
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

#ifndef SPROKIT_PROCESSES_GATE_PROCESS_H
#define SPROKIT_PROCESSES_GATE_PROCESS_H

#include "flow-config.h"

#include <sprokit/pipeline/process.h>

/**
 * \file gate_process.h
 *
 * \brief Declaration of the gate process.
 */

namespace sprokit
{

/**
 * \class gate_process
 *
 * \brief A process to route a data stream based on a test
 *
 * \process Routes incoming data
 *
 * \iports
 *
 * \iport{test} The test flag
 * \iport{input} Datum to route
 * \oports
 *
 * \oport{true} Datum passed here if \port{test} is true
 * \oport{false} Datum passed here if \port{test} is false
 *
 * \reqs
 *
 * \req The \port{test}  and \port{input} ports must be connected.
 *
 * \ingroup process_flow
 */
class SPROKIT_PROCESSES_FLOW_NO_EXPORT gate_process
  : public process
{
  public:
    /**
     * \brief Constructor.
     *
     * \param config The configuration for the process.
     */
    gate_process(kwiver::vital::config_block_sptr const& config);
    /**
     * \brief Destructor.
     */
    ~gate_process();
  protected:
    /**
     * \brief Step the process.
     */
    void _step();
  private:
    class priv;
    std::unique_ptr<priv> d;
};

}

#endif // SPROKIT_PROCESSES_PASS_PROCESS_H
