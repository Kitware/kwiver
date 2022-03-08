// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "transcode.h"

#include <vital/algo/video_input.h>
#include <vital/algo/video_output.h>

#include <vital/config/config_block_io.h>

#include <iostream>

namespace kv = kwiver::vital;
namespace kva = kwiver::vital::algo;

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
transcode_applet
::transcode_applet()
{}

// ----------------------------------------------------------------------------
void
transcode_applet
::add_command_options()
{
  m_cmd_options->add_options()                   //
    ( "h,help",   "Display applet usage.",
    ::cxxopts::value< bool >() )                 //
    ( "c,config", "Specify configuration file.",
    ::cxxopts::value< std::string >(), "file" )  //
    ( "i,input",  "Specify input video file.",
    ::cxxopts::value< std::string >(), "file" )  //
    ( "o,output", "Specify output video file.",
    ::cxxopts::value< std::string >(), "file" );
}

// ----------------------------------------------------------------------------
int
transcode_applet
::run()
{
  auto& cmd_args = command_args();

  // Parse help flag
  if( cmd_args[ "help" ].as< bool >() )
  {
    std::cerr << m_cmd_options->help();
    return EXIT_SUCCESS;
  }

  // Parse input argument
  if( !cmd_args.count( "input" ) )
  {
    std::cerr << "Specify input video file with -i/--input." << std::endl;
    return EXIT_FAILURE;
  }

  auto const& input_filename = cmd_args[ "input" ].as< std::string >();

  // Parse output argument
  if( !cmd_args.count( "output" ) )
  {
    std::cerr << "Specify output video file with -o/--output." << std::endl;
    return EXIT_FAILURE;
  }

  auto const& output_filename = cmd_args[ "output" ].as< std::string >();

  // Assemble configuration
  auto config = find_configuration( "applets/transcode.conf" );
  if( cmd_args.count( "config" ) )
  {
    auto const& config_filename = cmd_args[ "config" ].as< std::string >();
    config->merge_config( kv::read_config_file( config_filename ) );
  }

  // Setup video input
  kva::video_input_sptr input;
  kva::video_input
  ::set_nested_algo_configuration( "video_reader", config, input );
  kva::video_input
  ::get_nested_algo_configuration( "video_reader", config, input );
  input->open( input_filename );

  auto const video_settings = input->implementation_settings();

  // Setup video output
  kva::video_output_sptr output;
  kva::video_output
  ::set_nested_algo_configuration( "video_writer", config, output );
  kva::video_output
  ::get_nested_algo_configuration( "video_writer", config, output );
  output->open( output_filename, video_settings.get() );

  // Transcode frames
  kv::timestamp timestamp;
  for( input->next_frame( timestamp );
       !input->end_of_video();
       input->next_frame( timestamp ) )
  {
    auto const image = input->frame_image();
    for( auto const& metadata : input->frame_metadata() )
    {
      output->add_metadata( *metadata );
    }
    output->add_image( image, timestamp );
  }

  // Clean up
  input->close();
  output->close();

  return EXIT_SUCCESS;
}

} // namespace core

} // namespace arrows

} // namespace kwiver
