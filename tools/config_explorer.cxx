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

#include "config_explorer.h"

#include <vital/plugin_loader/plugin_manager.h>
#include <vital/config/config_block_io.h>
#include <vital/config/config_block.h>
#include <vital/config/config_parser.h>
#include <kwiversys/CommandLineArguments.hxx>

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>

typedef kwiversys::CommandLineArguments argT;

namespace kwiver {
namespace tools {

namespace {

  // Global options
  bool opt_detail_ds;
  bool opt_detail_dc;
  bool opt_help;
  std::vector< std::string > opt_path;
  std::string opt_app_name;
  std::string opt_app_version;
  std::string opt_install_prefix;

} // end namespace


// ------------------------------------------------------------------
void
config_explorer::
usage( std::ostream& outstream ) const
{
  outstream << "This program assists in debugging config loading problems. It loads a \n"
            << "configuration and displays the contents or displays the search path.\n"
            << "Additional paths can be specified in \"KWIVER_CONFIG_PATH\" environment variable\n"
            << "or on the command line with the -I or --path options.\n"
            << "\n"
            << "Usage: kwiver " << applet_name() << " <config-file-name> <options>\n"
            << "\n"
            << "Options are:\n"
            << "  -h / --help      displays usage information\n"
            << "  --path name      add directory to config search path(can appear multiple times)\n"
            << "  -Iname           add directory to config search path(can appear multiple times)\n"
            << "  -ds              generate detailed application-specific search paths\n"
            << "  -dc              generate detailed config contents output\n"
            << "  -a name          alternate application name\n"
            << "  -v version       optional application version string\n"
            << "  --prefix dir     optional non-standard install prefix directory\n"
            << "\n"
            << "If -ds is specified, the detailed search paths that apply to the application are\n"
            << "displayed only otherwise, the config file is loaded.\n"
            << "\n"
            << "The option -dc only has effect when a config file is specified and causes a\n"
            << "detailed output of the config entries.\n"
            << "\n"
            << "If -I or --path are specified, then the config file is only searched for using\n"
            << "the specified path. The application name based paths are not used.\n"
  ;

  return;
}


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


// ============================================================================
config_explorer::
config_explorer()
{ }


// ----------------------------------------------------------------------------
int
config_explorer::
run( const std::vector<std::string>& argv )
{
  opt_app_name = applet_name();

  kwiversys::CommandLineArguments arg;

  arg.Initialize( argv );
  arg.StoreUnusedArguments( true );

  arg.AddArgument( "-h",        argT::NO_ARGUMENT, &opt_help, "Display usage information" );
  arg.AddArgument( "--help",    argT::NO_ARGUMENT, &opt_help, "Display usage information" );

  // details
  arg.AddArgument( "-ds",       argT::NO_ARGUMENT, &opt_detail_ds, "Display detailed application search path" );
  arg.AddArgument( "-dc",       argT::NO_ARGUMENT, &opt_detail_dc, "Display detailed config contents" );

  // manual search path
  arg.AddCallback( "--path",    argT::SPACE_ARGUMENT, path_callback, 0, "Add directory to config search path" );
  arg.AddCallback( "-I",        argT::CONCAT_ARGUMENT, path_callback, 0, "Add directory to config search path" );

  // auto search path generation
  arg.AddArgument( "-a",        argT::SPACE_ARGUMENT, &opt_app_name, "Application name" );
  arg.AddArgument( "-v",        argT::SPACE_ARGUMENT, &opt_app_version, "Application version string" );
  arg.AddArgument( "--prefix",  argT::SPACE_ARGUMENT, &opt_install_prefix,
                   "Non-standard installation prefix. (e.g. /opt/kitware)" );

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

  //
  // Display application specific search path.
  //
  if ( opt_detail_ds )
  {
    kwiver::vital::config_path_list_t search_path =
      kwiver::vital::application_config_file_paths( opt_app_name,
                                                    opt_app_version,
                                                    opt_install_prefix );

    std::cout << "Application specific configuration search paths for\n"
              << "       App name: " << opt_app_name << std::endl
              << "    App version: " << opt_app_version << std::endl
              << " Install Prefix: " << opt_install_prefix << std::endl
              << std::endl;

    for( auto path : search_path )
    {
      std::cout << path << std::endl;
    }

    return EXIT_SUCCESS;
  }


  // Read in config
  if( newArgc <= 1 )
  {
    std::cout << "Missing file name.\n"
              << "Usage: " << newArgv[0] << " " << applet_name() << "  config-file-name\n"
              << "   " << newArgv[0] << " " << applet_name() << " --help for usage details\n"
              << std::endl;

    return EXIT_FAILURE;
  }

  const std::string config_file = newArgv[1];

  arg.DeleteRemainingArguments(newArgc, &newArgv);

  kwiver::vital::config_block_sptr config;

  if ( ! opt_path.empty() )
  {
    std::cout << "Using custom search path.\n";
    config = kwiver::vital::read_config_file( config_file,
                                              opt_path );
  }
  else
  {
    std::cout << "Using application default search path.\n";
    config = kwiver::vital::read_config_file( config_file,
                                              opt_app_name,
                                              opt_app_version,
                                              opt_install_prefix,
                                              true );  // merge all configs
  }

  //
  // Dump details of config
  //
  if ( opt_detail_dc )
  {
    std::cout << "Config contents for\n"
              << "       App name: " << opt_app_name << std::endl
              << "    App version: " << opt_app_version << std::endl
              << " Install Prefix: " << opt_install_prefix << std::endl
              << std::endl;

    kwiver::vital::write_config( config, std::cout );
  }

  return EXIT_SUCCESS;
} // run

} } // end namespace
