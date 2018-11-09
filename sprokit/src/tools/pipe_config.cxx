/*ckwg +29
 * Copyright 2012-2018 by Kitware, Inc.
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

#include "pipe_config.h"

#include "tool_support.h"
#include "tool_io.h"

#include <vital/config/config_block.h>

#include <sprokit/pipeline_util/export_pipe.h>

namespace sprokit {
namespace tools {

// ----------------------------------------------------------------------------
pipe_config::
pipe_config()
{
}

// ----------------------------------------------------------------------------
void
pipe_config::
usage( std::ostream& outstream ) const
{
  outstream << "This program configures the specified pipeline file.\n"
            << "Usage: " + applet_name() + " pipe-file [options]\n"
            << "\nOptions are:\n"
            << "     --help  |-h                 Output help message and quit.\n"
            << "     --config | -c   FILE        File containing supplemental configuration entries.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --setting | -s   VAR=VALUE  Additional configuration entries.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --include | -I   DIR        A directory to be added to configuration include path.\n"
            << "                                 Can occurr multiple times.\n"
            << "     --output | -o   PATH        Directory name for output files."
   ;
}


// ----------------------------------------------------------------------------
int
pipe_config::
run( const std::vector<std::string>& argv )
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
  if( options.remaining_argc <= 1 )
  {
    usage( std::cout );
    return EXIT_FAILURE;
  }

  // Load all known modules
  kwiver::vital::plugin_manager& vpm = kwiver::vital::plugin_manager::instance();
  vpm.load_all_plugins();

  // Add search path to builder.
  options.builder.add_search_path( options.opt_search_path );

  // Load the pipeline file.
  kwiver::vital::path_t const pipe_file( options.remaining_argv[1] );
  options.builder.load_pipeline( pipe_file );

  // Must be applied after pipe file is loaded.
  // To overwrite any existing settings
  options.add_options_to_builder();

  // Get handle to pipeline
  sprokit::pipeline_t const pipe = options.builder.pipeline();

  // get handle to config block
  kwiver::vital::config_block_sptr const conf = options.builder.config();

  if (!pipe)
  {
    std::cerr << "Error: Unable to bake pipeline" << std::endl;

    return EXIT_FAILURE;
  }

  sprokit::ostream_t const ostr = sprokit::open_ostream(options.opt_output);

  sprokit::export_pipe exp( options.builder );

  exp.generate( *ostr.get() );

  return EXIT_SUCCESS;
}

} } // end namespace
