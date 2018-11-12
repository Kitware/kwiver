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

#include "pipe_to_dot.h"

#include "tool_support.h"
#include "tool_io.h"

#include <sprokit/pipeline_util/export_dot.h>

#include <vital/config/config_block.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <sprokit/pipeline/pipeline.h>
#include <sprokit/pipeline/process.h>
#include <sprokit/pipeline/process_cluster.h>
#include <sprokit/pipeline/process_factory.h>
#include <sprokit/pipeline/types.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <cstddef>
#include <cstdlib>

namespace sprokit {
namespace tools {

// ----------------------------------------------------------------------------
pipe_to_dot::
pipe_to_dot()
{
}

// ----------------------------------------------------------------------------
void
pipe_to_dot::
usage( std::ostream& outstream ) const
{
  outstream << "This program runs the specified pipeline file.\n"
            << "Usage: " + applet_name() + " pipe-file [options]\n"
            << "\nOptions are:\n"
            << "     --help  | -h                Output help message and quit.\n"
            << "     --config | -c   FILE        File containing supplemental configuration entries.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --setting | -s   VAR=VALUE  Additional configuration entries.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --include | -I   DIR        A directory to be added to configuration include path.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --output | -o   PATH        Name for output files. '-' for stdout\n"
            << "     --cluster | -C  PATH        Cluster file to export.\n"
            << "     --cluster-type | T   OPT    Cluster type to export.\n"
            << "     --name | -n     NAME        Name of the graph.\n"
            << "     --setup                     Setup the pipeline before rendering.\n"
            << "     --link-prefix | -P   OPT    Prefix for links when formatting for sphinx\n"
  ;
}


// ----------------------------------------------------------------------------
int
pipe_to_dot::
run( const std::vector< std::string >& argv )
{
  tool_support options;

  options.init_args( argv );    // Add common options
  options.add_pipeline_output_args();

  if ( ! options.process_args() )
  {
    exit( 0 );
  }

  if ( options.opt_help )
  {
    usage( std::cout );
    return EXIT_SUCCESS;
  }

  // Check for required pipeline file
  if ( options.remaining_argc <= 1 )
  {
    usage( std::cout );
    return EXIT_FAILURE;
  }

  sprokit::process_cluster_t cluster;
  sprokit::pipeline_t pipe;

  bool const have_cluster = ! options.opt_cluster.empty();
  bool const have_cluster_type = ! options.opt_cluster_type.empty();
  bool const have_pipeline = ( nullptr != options.remaining_argv[1] );
  bool const have_setup = options.opt_setup_pipe;
  bool const have_link = ! options.opt_link_prefix.empty();

  bool const export_cluster = ( have_cluster || have_cluster_type );

  if ( export_cluster && have_pipeline )
  {
    std::cerr << "Error: The \'cluster\' and \'cluster-type\' options are "
                 "incompatible with the \'pipeline\' option" << std::endl;

    return EXIT_FAILURE;
  }

  if ( export_cluster && have_setup )
  {
    std::cerr << "Error: The \'cluster\' and \'cluster-type\' options are "
                 "incompatible with the \'setup\' option" << std::endl;

    return EXIT_FAILURE;
  }

  std::string const graph_name = options.opt_dot_name;

  if ( export_cluster )
  {
    if ( have_cluster && have_cluster_type )
    {
      std::cerr << "Error: The \'cluster\' option is incompatible "
                   "with the \'cluster-type\' option" << std::endl;

      return EXIT_FAILURE;
    }

    // Load all known modules
    kwiver::vital::plugin_manager& vpm = kwiver::vital::plugin_manager::instance();
    vpm.load_all_plugins();


    //// new
    // Add search path to builder.
    options.builder.add_search_path( options.opt_search_path );

    // Load the pipeline file.
    kwiver::vital::path_t const pipe_file( options.remaining_argv[1] );
    options.builder.load_pipeline( pipe_file );

    // Must be applied after pipe file is loaded.
    // To overwrite any existing settings
    options.add_options_to_builder();

    // get handle to config block
    kwiver::vital::config_block_sptr const conf = options.builder.config();

    /// end new


    if ( have_cluster )
    {
      sprokit::istream_t const istr = sprokit::open_istream( options.opt_cluster );

      sprokit::pipeline_builder builder;
      builder.load_cluster( *istr );
      sprokit::cluster_info_t const info = builder.cluster_info();

      conf->set_value( sprokit::process::config_name, graph_name );

      sprokit::process_t const proc = info->ctor( conf );
      cluster = std::dynamic_pointer_cast< sprokit::process_cluster > ( proc );
    }
    else if ( have_cluster_type )
    {
      sprokit::process::type_t const type = options.opt_cluster_type;

      sprokit::process_t const proc = sprokit::create_process( type, graph_name, conf );
      cluster = std::dynamic_pointer_cast< sprokit::process_cluster > ( proc );

      if ( ! cluster )
      {
        std::cerr << "Error: The given type (\'" << type << "\') "
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
  else if ( have_pipeline )
  {
    // Add search path to builder.
    options.builder.add_search_path( options.opt_search_path );

    // Load the pipeline file.
    kwiver::vital::path_t const pipe_file( options.remaining_argv[1] );
    options.builder.load_pipeline( pipe_file );

    // Must be applied after pipe file is loaded.
    // To overwrite any existing settings
    options.add_options_to_builder();

    // Get handle to pipeline
    pipe = options.builder.pipeline();

    if ( ! pipe )
    {
      std::cerr << "Error: Unable to bake pipeline" << std::endl;

      return EXIT_FAILURE;
    }
  }
  else
  {
    std::cerr << "Error: One of \'cluster\', \'cluster-type\', or "
                 "\'pipeline\' must be specified" << std::endl;

    this->usage( std::cerr );
  }

  // Make sure we have one, but not both.
  if ( ! cluster == ! pipe )
  {
    std::cerr << "Internal error: option tracking failure" << std::endl;

    return EXIT_FAILURE;
  }

  sprokit::ostream_t const ostr = sprokit::open_ostream( options.opt_output );

  if ( cluster )
  {
    sprokit::export_dot( *ostr, cluster, graph_name );
  }
  else if ( pipe )
  {
    if ( have_setup )
    {
      pipe->setup_pipeline();
    }

    if ( have_link )
    {
      sprokit::export_dot( *ostr, pipe, graph_name, options.opt_link_prefix );
    }
    else
    {
      sprokit::export_dot( *ostr, pipe, graph_name );
    }
  }

  return EXIT_SUCCESS;
} // pipe_to_dot::run


}
}   // end namespace
