/*ckwg +29
 * Copyright 2011-2019 by Kitware, Inc.
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

#include "pipeline_runner.h"

#include <vital/config/config_block.h>
#include <vital/config/config_block_formatter.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <sprokit/pipeline/scheduler.h>
#include <sprokit/pipeline/scheduler_factory.h>
#include <sprokit/pipeline/pipeline.h>
#include <sprokit/pipeline_util/pipeline_builder.h>
#include <sprokit/pipeline_util/pipe_display.h>

#include <cstdlib>
#include <iostream>

namespace sprokit {
namespace tools {

static const auto scheduler_block = kwiver::vital::config_block_key_t("_scheduler");


// ----------------------------------------------------------------------------
pipeline_runner::
pipeline_runner()
{
}


// ----------------------------------------------------------------------------
void
pipeline_runner::
add_command_options()
{
  m_cmd_options->custom_help( wrap_text( "[options] pipe-file\n"
           "This program runs the specified pipeline file."
                                ));

  m_cmd_options->positional_help( "\n  pipe-file  - name of pipeline file." );

  m_cmd_options->add_options()
    ( "h,help", "Display applet usage" );

  m_cmd_options->add_options("pipe")
    ( "c,config", "File name containing supplemental configuration entries. Can occur multiple times.",
      cxxopts::value<std::vector<std::string>>() )
    ( "s,setting", "Additional configuration entries in the form of VAR=VALUE. "
      "Can occur multiple times",
      cxxopts::value<std::vector<std::string>>() )
    ( "I,include", "A directory to be added to configuration include path. Can occur multiple times.",
      cxxopts::value<std::vector<std::string>>()  )
    ( "S,scheduler", "Scheduler type to use.", cxxopts::value<std::string>() )
    ( "D,dump-pipe", "Dump final pipeline configuration. This is useful for "
      "debugging config related problems." )
    ;

    // positional parameters
  m_cmd_options->add_options()
    ( "p,pipe-file", "Input pipeline file", cxxopts::value<std::string>())
    ;

  m_cmd_options->parse_positional("pipe-file");
}

// ----------------------------------------------------------------------------
int
pipeline_runner::
run()
{
  const std::string opt_app_name = applet_name();

  auto& cmd_args = command_args();

  if ( cmd_args["help"].as<bool>() )
  {
    std::cout << m_cmd_options->help();
    return EXIT_SUCCESS;
  }

  // Load all known modules
  kwiver::vital::plugin_manager& vpm = kwiver::vital::plugin_manager::instance();
  vpm.load_all_plugins();

  sprokit::pipeline_builder builder;

  // Add search path to builder.
  if ( cmd_args.count("include") > 0 )
  {
    builder.add_search_path( cmd_args["include"].as<std::vector<std::string>>() );
  }

  if ( cmd_args.count("pipe-file") == 0 )
  {
    // error & exit
    std::cerr << "Required pipeline file missing\n "
              << m_cmd_options->help();
    return EXIT_FAILURE;
  }

  // Load the pipeline file.
  kwiver::vital::path_t const pipe_file( cmd_args["pipe-file"].as<std::string>() );
  builder.load_pipeline( pipe_file );

  // Must be applied after pipe file is loaded.
  // To overwrite any existing settings
  if ( cmd_args.count("config") > 0 )
  {
    std::vector< std::string > config_file_names = cmd_args["config"].as<std::vector<std::string>>();
    for ( const auto& config : config_file_names )
    {
      builder.load_supplement( config );
    }
  }

  // Add accumulated settings to the pipeline
  if ( cmd_args.count("setting") > 0 )
  {
    std::vector< std::string > config_settings = cmd_args["setting"].as<std::vector<std::string>>();
    for ( const auto& setting : config_settings )
    {
      builder.add_setting( setting );
    }
  }

  // Get handle to pipeline
  sprokit::pipeline_t const pipe = builder.pipeline();

  // get handle to config block
  kwiver::vital::config_block_sptr const conf = builder.config();

  // nice to dump config at this point
  if ( cmd_args["dump-pipe"].as<bool>() )
  {
    std::cout << "\nPipeline contents:\n";;
    sprokit::pipe_display pd( std::cout );
    pd.print_loc();
    pd.display_pipe_blocks( builder.pipeline_blocks() );

    return EXIT_SUCCESS;
  }

  if (!pipe)
  {
    std::cerr << "Error: Unable to bake pipeline" << std::endl;
    return EXIT_FAILURE;
  }

  // Get pipeline ready to run
  pipe->setup_pipeline();

  //
  // Check for scheduler specification in config block
  //
  auto scheduler_type = sprokit::scheduler_factory::default_type;

  // Check if scheduler type was on the command line.
  if ( cmd_args.count("scheduler") > 0 )
  {
    scheduler_type = cmd_args["scheduler"].as<std::string>();
  }
  else
  {
    scheduler_type = conf->get_value(
        scheduler_block + kwiver::vital::config_block::block_sep()
        + "type",  // key string
        sprokit::scheduler_factory::default_type ); // default value
  }

  // Get scheduler sub block based on selected scheduler type
  kwiver::vital::config_block_sptr const scheduler_config =
             conf->subblock(scheduler_block +
                            kwiver::vital::config_block::block_sep() +
                            scheduler_type);

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
