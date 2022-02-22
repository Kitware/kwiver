// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "pipe_to_dot.h"

#include "tool_io.h"

#include <sprokit/pipeline_util/export_dot.h>

#include <vital/config/config_block.h>
#include <vital/config/config_block_io.h>
#include <vital/plugin_loader/plugin_manager.h>
#include <vital/util/get_paths.h>

#include <sprokit/pipeline/pipeline.h>
#include <sprokit/pipeline/process.h>
#include <sprokit/pipeline/process_cluster.h>
#include <sprokit/pipeline/process_factory.h>
#include <sprokit/pipeline/types.h>
#include <sprokit/pipeline_util/pipeline_builder.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdlib>

namespace sprokit {

namespace tools {

// ----------------------------------------------------------------------------
pipe_to_dot
::pipe_to_dot()
{
}

// ----------------------------------------------------------------------------
void
pipe_to_dot
::add_command_options()
{
  m_cmd_options->custom_help(
    wrap_text( "[options]\n"
               "This program generates a DOT file from the pipeline topology."
               )
    );

  m_cmd_options->add_options( "pipe" )(
    "c,config",
    "File containing supplemental configuration entries. Can occur multiple times.",
    cxxopts::value< std::vector< std::string > >() )(
    "s,setting",
    "Additional configuration entries in the form of VAR=VALUE. "
    "Can occur multiple times",
    cxxopts::value< std::vector< std::string > >() )(
    "I,include",
    "A directory to be added to configuration include path. Can occur multiple times.",
    cxxopts
    ::value< std::vector< std::string > >() )(
    "setup",
    "Setup pipeline before rendering" );

  m_cmd_options->add_options( "input" )(
    "p,pipe-file", "Input pipeline file file",
    cxxopts::value< std::string >() )(
    "C,cluster", "Cluster file to export",
    cxxopts::value< std::string >() )(
    "T,cluster-type", "Cluster type to export",
    cxxopts::value< std::string >() );

  m_cmd_options->add_options( "output" )(
    "n,name", "Name of the graph",
    cxxopts::value< std::string >()->default_value( "unnamed" ) )(
    "o,output", "Name of output file or '-' for stdout.",
    cxxopts::value< std::string >()->default_value( "-" ) )(
    "P,link-prefix", "Prefix for links when formatting for sphinx",
    cxxopts::value< std::string >() );
}

// ----------------------------------------------------------------------------
int
pipe_to_dot
::run()
{
  const std::string opt_app_name = applet_name();

  auto& cmd_args = command_args();

  if( cmd_args.count( "pipe-file" ) == 0 )
  {
    // error & exit
    std::cout   << "Required pipeline file missing\n "
                << m_cmd_options->help();
    return EXIT_SUCCESS;
  }

  sprokit::process_cluster_t cluster;
  sprokit::pipeline_t pipe;

  bool const have_cluster = ( cmd_args[ "cluster" ].count() > 0 );
  bool const have_cluster_type = ( cmd_args[ "cluster-type" ].count() > 0 );
  bool const have_pipeline = ( cmd_args.count( "pipe-file" ) > 0 );
  bool const have_setup = cmd_args[ "setup" ].as< bool >();
  bool const have_link = ( cmd_args.count( "link-prefix" ) > 0 );

  bool const export_cluster = ( have_cluster || have_cluster_type );

  if( export_cluster && have_pipeline )
  {
    std::cerr << "Error: The \'cluster\' and \'cluster-type\' options are "
                 "incompatible with the \'pipeline\' option" << std::endl;

    return EXIT_FAILURE;
  }

  if( export_cluster && have_setup )
  {
    std::cerr << "Error: The \'cluster\' and \'cluster-type\' options are "
                 "incompatible with the \'setup\' option" << std::endl;

    return EXIT_FAILURE;
  }

  const std::string graph_name = cmd_args[ "name" ].as< std::string >();

  sprokit::pipeline_builder builder;

  if( export_cluster )
  {
    if( have_cluster && have_cluster_type )
    {
      std::cerr << "Error: The \'cluster\' option is incompatible "
                   "with the \'cluster-type\' option" << std::endl;

      return EXIT_FAILURE;
    }

    // Load all known modules
    kwiver::vital::plugin_manager& vpm =
      kwiver::vital::plugin_manager::instance();
    vpm.load_all_plugins();

    // Add user-provided paths
    if( cmd_args.count( "include" ) > 0 )
    {
      builder.add_search_path(
          cmd_args[ "include" ].as< std::vector< std::string > >() );
    }

    // Add standard search locations
    const std::string prefix = kwiver::vital::get_executable_path() + "/..";
    builder.add_search_path( kwiver::vital::kwiver_config_file_paths( prefix ) );

    // Load the pipeline file.
    kwiver::vital::path_t const pipe_file(
      cmd_args[ "pipe-file" ].as< std::string >() );
    builder.load_pipeline( pipe_file );

    // Must be applied after pipe file is loaded.
    // To overwrite any existing settings
    if( cmd_args.count( "config" ) > 0 )
    {
      std::vector< std::string > config_file_names =
        cmd_args[ "config" ].as< std::vector< std::string > >();

      for( auto config : config_file_names )
      {
        builder.load_supplement( config );
      }
    }

    // Add accumulated settings to the pipeline
    if( cmd_args.count( "setting" ) > 0 )
    {
      std::vector< std::string > config_settings =
        cmd_args[ "setting" ].as< std::vector< std::string > >();

      for( auto setting : config_settings )
      {
        builder.add_setting( setting );
      }
    }

    // get handle to config block
    kwiver::vital::config_block_sptr const conf = builder.config();

    if( have_cluster )
    {
      sprokit::istream_t const istr =
        sprokit::open_istream( cmd_args[ "cluster" ].as< std::string >() );

      builder.load_cluster( *istr );

      sprokit::cluster_info_t const info = builder.cluster_info();

      conf->set_value( sprokit::process::config_name, graph_name );

      sprokit::process_t const proc = info->ctor( conf );
      cluster = std::dynamic_pointer_cast< sprokit::process_cluster >( proc );
    }
    else if( have_cluster_type )
    {
      sprokit::process::type_t const type =
        cmd_args[ "cluster-type" ].as< std::string >();

      sprokit::process_t const proc = sprokit::create_process(
        type, graph_name, conf );
      cluster = std::dynamic_pointer_cast< sprokit::process_cluster >( proc );

      if( !cluster )
      {
        std::cerr << "Error: The given type (\'" << type << "\') " <<
          "is not a cluster" << std::endl;

        return EXIT_FAILURE;
      }
    }
    else
    {
      std::cerr << "Internal error: option tracking failure" << std::endl;

      return EXIT_FAILURE;
    }
  }
  else if( have_pipeline )
  {
    // Add search path to builder.
    if( cmd_args.count( "include" ) > 0 )
    {
      builder.add_search_path(
          cmd_args[ "include" ].as< std::vector< std::string > >() );
    }

    // Load the pipeline file.
    kwiver::vital::path_t const pipe_file(
      cmd_args[ "pipe-file" ].as< std::string >() );

    builder.load_pipeline( pipe_file );

    // Must be applied after pipe file is loaded.
    // To overwrite any existing settings
    if( cmd_args.count( "config" ) > 0 )
    {
      std::vector< std::string > config_file_names =
        cmd_args[ "config" ].as< std::vector< std::string > >();

      for( auto config : config_file_names )
      {
        builder.load_supplement( config );
      }
    }

    // Add accumulated settings to the pipeline
    if( cmd_args.count( "setting" ) > 0 )
    {
      std::vector< std::string > config_settings =
        cmd_args[ "setting" ].as< std::vector< std::string > >();
      for( auto setting : config_settings )
      {
        builder.add_setting( setting );
      }
    }

    // Get handle to pipeline
    pipe = builder.pipeline();

    if( !pipe )
    {
      std::cerr << "Error: Unable to bake pipeline" << std::endl;

      return EXIT_FAILURE;
    }
  }
  else
  {
    std::cerr   << "Error: One of \'cluster\', \'cluster-type\', or "
                << "\'pipeline\' must be specified" << std::endl
                << m_cmd_options->help();
  }

  // Make sure we have one, but not both.
  if( !cluster == !pipe )
  {
    std::cerr << "Internal error: option tracking failure" << std::endl;

    return EXIT_FAILURE;
  }

  sprokit::ostream_t const ostr = sprokit::open_ostream(
    cmd_args[ "output" ].as< std::string >() );

  if( cluster )
  {
    sprokit::export_dot( *ostr, cluster, graph_name );
  }
  else if( pipe )
  {
    if( have_setup )
    {
      pipe->setup_pipeline();
    }

    if( have_link )
    {
      sprokit::export_dot( *ostr, pipe, graph_name,
                           cmd_args[ "link-prefix" ].as< std::string >() );
    }
    else
    {
      sprokit::export_dot( *ostr, pipe, graph_name );
    }
  }

  return EXIT_SUCCESS;
}

} // namespace tools

} // namespace sprokit
