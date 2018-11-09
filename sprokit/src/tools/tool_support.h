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

#ifndef SPROKIT_TOOLS_TOOL_SUPPORT_H
#define SPROKIT_TOOLS_TOOL_SUPPORT_H

#include <kwiversys/CommandLineArguments.hxx>

#include <sprokit/pipeline_util/pipeline_builder.h>

#include <vector>
#include <string>
#include <sstream>

namespace sprokit {

class pipeline_builder;

using argT = kwiversys::CommandLineArguments;

// -----------------------------------------------------------------
/** Class to support sprokit tools options
 *
 * This class contains a bunch of support methods and dara used by the
 * sprokit tools. It is a super-set of all items needed by all of the
 * tools, so for some tools, there may be some elements that are not
 * used.
 */
class tool_support
{
public:
  tool_support();
  ~tool_support() = default;


  void init_args( const std::vector< std::string >& argv);
  void add_pipeline_output_args();
  void add_pipeline_run_options();
  void add_pipeline_dot_args();

  bool process_args();
  void add_options_to_builder( );

 // ============================================================================
  bool opt_help { false };
  bool opt_setup_pipe { false };

  std::string opt_app_name;
  std::string opt_scheduler;    // name of scheduler
  std::string opt_config_name;  // name of config file
  std::string opt_output;       // filename for output
  std::string opt_cluster;
  std::string opt_cluster_type;
  std::string opt_dot_name;
  std::string opt_link_prefix;

  std::vector< std::string > config_settings;
  std::vector< std::string > config_file_names;
  std::vector< std::string > opt_search_path;

  kwiversys::CommandLineArguments command_args;

  char** remaining_argv { 0 };
  int remaining_argc { 0 };

  sprokit::pipeline_builder builder;


private:
  static int path_callback( const char*  argument, // name of argument
                            const char*  value,    // value of argument
                            void*        call_data ); // data from register call

  static int config_callback( const char*  argument, // name of argument
                              const char*  value,    // value of argument
                              void*        call_data ); // data from register call

  static int setting_callback( const char* argument,  // name of argument
                               const char* value,     // value of argument
                               void*       call_data ); // data from register call

}; // end class tool_support


} // end namespace

#endif /* SPROKIT_TOOLS_TOOL_SUPPORT_H */
