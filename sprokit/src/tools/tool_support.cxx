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

#include "tool_support.h"

#include <sprokit/pipeline_util/pipeline_builder.h>

#include <iostream>

namespace sprokit {

tool_support::
tool_support() { }


// ------------------------------------------------------------------
int
tool_support::
path_callback( const char*  argument, // name of argument
                 const char*  value,    // value of argument
                 void*        call_data ) // data from register call
{
  const std::string p( value );
  tool_support* p_obj = static_cast< tool_support* >(call_data);

  p_obj->opt_search_path.push_back( p );
  return 1;   // return true for OK
}


// ------------------------------------------------------------------
int
tool_support::
config_callback( const char*  argument, // name of argument
                   const char*  value,    // value of argument
                   void*        call_data ) // data from register call
{
  const std::string p( value );
  tool_support* p_obj = static_cast< tool_support* >(call_data);

  p_obj->config_file_names.push_back( p );
  return 1;   // return true for OK
}


// ------------------------------------------------------------------
int
tool_support::
setting_callback( const char* argument,  // name of argument
                    const char* value,     // value of argument
                    void*       call_data ) // data from register call
{
  const std::string p( value );
  tool_support* p_obj = static_cast< tool_support* >(call_data);

  p_obj->config_settings.push_back( p );
  return 1;   // return true for OK
}


// ----------------------------------------------------------------------------
void
tool_support::
init_args( const std::vector< std::string >& argv )
{
  command_args.Initialize( argv );
  command_args.StoreUnusedArguments( true );

  command_args.AddArgument( "-h",          argT::NO_ARGUMENT, &this->opt_help, "Display usage information" );
  command_args.AddArgument( "--help",      argT::NO_ARGUMENT, &this->opt_help, "Display usage information" );

  command_args.AddArgument( "-c",          argT::SPACE_ARGUMENT, &this->opt_config_name, "Config file name" );
  command_args.AddArgument( "--config",    argT::SPACE_ARGUMENT, &this->opt_config_name, "Config file name" );

  command_args.AddCallback( "--path",      argT::SPACE_ARGUMENT, &this->path_callback, this, "Add directory to search path" );
  command_args.AddCallback( "-I",          argT::SPACE_ARGUMENT, &this->path_callback, this, "Add directory to search path" );

  command_args.AddCallback( "-s",          argT::SPACE_ARGUMENT, &this->setting_callback, this, "Add config setting" );
  command_args.AddCallback( "--setting",   argT::SPACE_ARGUMENT, &this->setting_callback, this, "Add config setting" );
}


// ----------------------------------------------------------------------------
void
tool_support::
add_pipeline_output_args()
{
  command_args.AddArgument( "-o",          argT::NO_ARGUMENT, &this->opt_output, "Output path" );
  command_args.AddArgument( "--output",    argT::NO_ARGUMENT, &this->opt_output, "Output path" );
}


// ----------------------------------------------------------------------------
void
tool_support::
add_pipeline_run_options()
{
  command_args.AddArgument( "-S",          argT::SPACE_ARGUMENT, &this->opt_scheduler, "Scheduler name" );
  command_args.AddArgument( "--scheduler", argT::SPACE_ARGUMENT, &this->opt_scheduler, "Scheduler name" );
}

// ----------------------------------------------------------------------------
void
tool_support::
add_pipeline_dot_args()
{
  command_args.AddArgument( "-C",
                            argT::SPACE_ARGUMENT,
                            &this->opt_cluster,
                            "Cluster file to export" );
  command_args.AddArgument( "--cluster",
                            argT::SPACE_ARGUMENT,
                            &this->opt_cluster,
                            "Cluster file to export" );
  command_args.AddArgument( "-T",
                            argT::SPACE_ARGUMENT,
                            &this->opt_cluster_type,
                            "Cluster type to export" );
  command_args.AddArgument( "--cluster-type",
                            argT::SPACE_ARGUMENT,
                            &this->opt_cluster_type,
                            "Cluster type to export" );
  command_args.AddArgument( "-n",
                            argT::SPACE_ARGUMENT,
                            &this->opt_dot_name,
                            "Name of the graph" );
  command_args.AddArgument( "--name",
                            argT::SPACE_ARGUMENT,
                            &this->opt_dot_name,
                            "Name of the graph" );
  command_args.AddArgument( "-P",
                            argT::SPACE_ARGUMENT,
                            &this->opt_link_prefix,
                            "Prefix for links when formatting for sphinx" );
  command_args.AddArgument( "--link-prefix",
                            argT::SPACE_ARGUMENT,
                            &this->opt_link_prefix,
                            "Prefix for links when formatting for sphinx" );
  command_args.AddArgument( "--setup",
                            argT::NO_ARGUMENT,
                            &this->opt_setup_pipe,
                            "Setup pipeline before rendering.");

}


// ----------------------------------------------------------------------------
bool
tool_support::
process_args()
{
  if ( ! command_args.Parse() )
  {
    std::cerr << "Problem parsing arguments" << std::endl;
    return false;
  }

  command_args.GetUnusedArguments( &remaining_argc, &remaining_argv );

  return true;
}


// ----------------------------------------------------------------------------
void
tool_support::
add_options_to_builder()
{
  // Add accumulated config files
  for ( auto config : config_file_names )
  {
    builder.load_supplement( config );
  }

  // Add accumulated settings to the pipeline
  for ( auto setting : config_settings )
  {
    builder.add_setting( setting );
  }
}

} // end namespace
