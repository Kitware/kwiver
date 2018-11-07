/*ckwg +29
 * Copyright 2011-2018 by Kitware, Inc.
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

#include "pipeline_run_options.h"

#include <vital/config/config_block.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <sprokit/tools/tool_main.h>
#include <sprokit/tools/tool_usage.h>
#include <sprokit/tools/build_pipeline_from_options.h>

#include <sprokit/pipeline/scheduler.h>
#include <sprokit/pipeline/scheduler_factory.h>
#include <sprokit/pipeline/pipeline.h>

#include <kwiversys/CommandLineArguments.hxx>

#include <cstdlib>

typedef kwiversys::CommandLineArguments argT;

namespace kwiver {
namespace tools {

static const auto scheduler_block = kwiver::vital::config_block_key_t("_scheduler");

namespace {

  // Global options
  bool opt_help;
  std::vector< std::string > opt_path;
  std::string opt_app_name;
  std::string opt_scheduler;

  build_pipeline_options pipe_options;
} // end namespace


// ------------------------------------------------------------------
static int
path_callback( const char*  argument,   // name of argument
               const char*  value,      // value of argument
               void*        call_data ) // data from register call
{
  const std::string p( value );
  opt_path.push_back( p );
  return 1;   // return true for OK
}


// ------------------------------------------------------------------
static int
config_callback( const char*  argument,   // name of argument
                 const char*  value,      // value of argument
                 void*        call_data ) // data from register call
{
  const std::string p( value );
  pipe_options.config_file_names.push_back( p );
  return 1;   // return true for OK
}


// ------------------------------------------------------------------
static int
setting_callback( const char*  argument,   // name of argument
                  const char*  value,      // value of argument
                  void*        call_data ) // data from register call
{
  const std::string p( value );
  pipe_options.config_settings.push_back( p );
  return 1;   // return true for OK
}


// ----------------------------------------------------------------------------
void
pipeline_runner::
usage( std::ostream& outstream ) const
{
  outstream << "This program runs the specified pipeline file.\n"
            << "Usage: " << applet_name() << " pipe-file [options]" << std::endl
            << "\n"
            << "Options are:\n"
            << "     --help / -h                 Output help message and quit.\n"
            << "     --config / -c   FILE        File containing supplemental configuration entries.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --setting / -s   VAR=VALUE  Additional configuration entries.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --include / -I   DIR        A directory to be added to configuration include path.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --scheduler / -S   TYPE     Scheduler type to use.\n"
    ;
}


// ----------------------------------------------------------------------------
int
pipeline_runner::
run( const std::vector<std::string>& argv )
{
  opt_app_name = applet_name();

  kwiversys::CommandLineArguments arg;

  arg.Initialize( argv );
  arg.StoreUnusedArguments( true );

  arg.AddArgument( "-h",          argT::NO_ARGUMENT, &opt_help, "Display usage information" );
  arg.AddArgument( "--help",      argT::NO_ARGUMENT, &opt_help, "Display usage information" );

  arg.AddArgument( "-c",          argT::SPACE_ARGUMENT, &opt_config_name, "Config file name" );
  arg.AddArgument( "--config",    argT::SPACE_ARGUMENT, &opt_config_name, "Config file name" );

  arg.AddCallback( "--path",      argT::SPACE_ARGUMENT, path_callback, 0, "Add directory to search path" );
  arg.AddCallback( "-I",          argT::SPACE_ARGUMENT, path_callback, 0, "Add directory to search path" );

  arg.AddArgument( "-S",          argT::SPACE_ARGUMENT, &opt_scheduler, "Scheduler name" );
  arg.AddArgument( "--scheduler", argT::SPACE_ARGUMENT, &opt_scheduler, "Scheduler name" );

  arg.AddArgument( "-s",          argT::SPACE_ARGUMENT, setting_callback, "Add config setting" );
  arg.AddArgument( "--setting",   argT::SPACE_ARGUMENT, setting_callback, "Add config setting" );

  if ( ! arg.Parse() )
  {
    std::cerr << "Problem parsing arguments" << std::endl;
    exit( 0 );
  }

  if ( opt_help )
  {
    usage( std::cout );
    return EXIT_SUCCESS;
  }

  char** newArgv = 0;
  int newArgc = 0;
  arg.GetUnusedArguments(&newArgc, &newArgv);

  // Check for required pipeline file
  if( newArgc <= 1 )
  {
    usage( std::cout );
    return EXIT_FAILURE;
  }

  // Load all known modules
  kwiver::vital::plugin_manager& vpm = kwiver::vital::plugin_manager::instance();
  vpm.load_all_plugins();

  //+ const sprokit::build_pipeline_from_options builder(vm, desc);
  //@todo Refactor (pull-up) this code when more tools that work with pipelines are completed.
  //
  // Options:
  //
  // +1) Create a new class that contains the config settings vector and
  // config file vector. Pass that to a new build_pipeline_from_options class.
  //
  // 2) Make a subclass of applet that adds the config file and
  // settings handling.
  //
  const sprokit::pipeline_builder builder();

  kwiver::vital::path_t const ipath( newArgv[1] );
  builder->load_pipeline( ipath );

  // Add accumulated config files
  for ( auto config : opt_config_name )
  {
    builder->load_supplement( config );
  }

  // Add accumulated settings to the pipeline
  for ( auto setting : opt_settings )
  {
    builder->add_setting( setting );
  }
  //+ end of refactor candidate

  //@bug AFIK the config path specified via the command line options
  // has never been used to help locate pipeline files.
  // Maybe the path should be added to the builder before the call
  // to load_pipeline()?
  //
  // Currently, the parser gets the augmented search path and the
  // parser is local to the load_pipe_...() calls

  // Get handle to pipeline
  sprokit::pipeline_t const pipe = builder.pipeline();

  // get handle to config block
  kwiver::vital::config_block_sptr const conf = builder.config();

  if (!pipe)
  {
    std::cerr << "Error: Unable to bake pipeline" << std::endl;
    return EXIT_FAILURE;
  }

  pipe->setup_pipeline();

  //
  // Check for scheduler specification in config block
  //
  auto scheduler_type = sprokit::scheduler_factory::default_type;

  if (vm.count("scheduler"))
  {
    scheduler_type = vm["scheduler"].as<sprokit::scheduler::type_t>();
  }
  else
  {
    scheduler_type = conf->get_value(
        scheduler_block + kwiver::vital::config_block::block_sep + "type",  // key string
        sprokit::scheduler_factory::default_type ); // default value
  }

  // Get scheduler sub block based on selected scheduler type
  kwiver::vital::config_block_sptr const scheduler_config = conf->subblock(scheduler_block +
                                              kwiver::vital::config_block::block_sep + scheduler_type);

  sprokit::scheduler_t scheduler = sprokit::create_scheduler(scheduler_type, pipe, scheduler_config);

  if (!scheduler)
  {
    std::cerr << "Error: Unable to create scheduler" << std::endl;

    return EXIT_FAILURE;
  }

  scheduler->start();
  scheduler->wait();

  return EXIT_SUCCESS;
}

} } // end namespace
