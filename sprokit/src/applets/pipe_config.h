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

#ifndef KWIVER_TOOL_PIPE_CONFIG_H
#define KWIVER_TOOL_PIPE_CONFIG_H

#include <vital/applets/kwiver_applet.h>

#include <string>
#include <vector>

namespace sprokit {
namespace tools {

class pipe_config
  : public kwiver::tools::kwiver_applet
{
public:
  pipe_config();

  virtual int run() override;
  virtual void add_command_options() override;

  PLUGIN_INFO( "pipe-config",
    "Configures a pipeline\n\n"
    "This tool reads a pipeline configuration file, applies the program options "
    "and generates a \"compiled\" config file. "
    "At its most basic, this tool will validate a pipeline "
    "configuration, but it does so much more.  Specific pipeline "
    "configurations can be generated from generic descriptions. "
    "\n\n"
    "Global config sections can ge inserted in the resulting configuration "
    "file with the --setting option, with multiple options allowed on the "
    "command line. For example, --setting master:value=FOO will generate a "
    "config section: "
    "\n\n"
    "config master\n"
    "  :value FOO\n"
    "\n\b"
    "The --config option specifies a file that contains additional "
    "configuration parameters to be merged into the generated "
    "configuration. "
    "\n\n"
    "Use the --include option to add additional directories to search for "
    "included configuration files. "
    "\n\n"
    "The --pipeline option specifies the file that contains the main pipeline specification"
    );

}; // end of class

} } // end namespace

#endif /* KWIVER_TOOL_PIPE_CONFIG_H */
