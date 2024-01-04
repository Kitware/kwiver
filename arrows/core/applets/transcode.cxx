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

namespace {

// ----------------------------------------------------------------------------
void
check_input( kva::video_input_sptr const& input,
             cxxopts::ParseResult const& cmd_args )
{
  // Check capabilities
  auto const& capabilities = input->get_implementation_capabilities();
  if( cmd_args.count( "copy-video" ) &&
      !capabilities.has_capability( kva::video_input::HAS_RAW_IMAGE ) )
  {
    std::cerr << "--copy-video: Video input `" << input->impl_name()
              << "` does not have this capability." << std::endl;
    exit( EXIT_FAILURE );
  }
  if( cmd_args.count( "copy-metadata" ) &&
      !capabilities.has_capability( kva::video_input::HAS_RAW_METADATA ) )
  {
    std::cerr << "--copy-metadata: Video input `" << input->impl_name()
              << "` does not have this capability." << std::endl;
    exit( EXIT_FAILURE );
  }
}

// ----------------------------------------------------------------------------
void
check_output( kva::video_output_sptr const& output,
              cxxopts::ParseResult const& cmd_args )
{
  // Check initialization
  if( !output )
  {
    std::cerr << "Failed to initialize video output." << std::endl;
    exit( EXIT_FAILURE );
  }
}

} // namespace <anonymous>

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
    ::cxxopts::value< std::string >(), "file" )  //
    ( "copy-video", "Directly copy raw video without modification." )
    ( "copy-metadata", "Directly copy raw metadata without modification." );
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

  // Check initialization
  if( !input )
  {
    std::cerr << "Failed to initialize video input." << std::endl;
    exit( EXIT_FAILURE );
  }

  try
  {
    input->open( input_filename );
  }
  catch( kv::video_runtime_exception const& e )
  {
    std::cerr << e.what() << std::endl;
    exit( EXIT_FAILURE );
  }
  catch( kv::file_not_found_exception const& e )
  {
    std::cerr << e.what() << std::endl;
    exit( EXIT_FAILURE );
  }
  check_input( input, cmd_args );

  // Acquire first frame, which may help produce more accurate video settings
  kv::timestamp timestamp;
  input->next_frame( timestamp );
  auto const video_settings = input->implementation_settings();

  // Setup video output
  kva::video_output_sptr output;
  kva::video_output
  ::set_nested_algo_configuration( "video_writer", config, output );
  kva::video_output
  ::get_nested_algo_configuration( "video_writer", config, output );
  check_output( output, cmd_args );
  output->open( output_filename, video_settings.get() );

  // Transcode frames
  for( ; !input->end_of_video(); input->next_frame( timestamp ) )
  {
    // Transcode metadata
    if( cmd_args.count( "copy-metadata" ) )
    {
      auto const md = input->raw_frame_metadata();
      if( !md )
      {
        std::cerr << "No raw metadata found for frame " << timestamp.get_frame()
                  << "." << std::endl;
        exit(-1);
      }
      output->add_metadata( *md );
    }
    else
    {
      for( auto const& metadata : input->frame_metadata() )
      {
        output->add_metadata( *metadata );
      }
    }

    // Transcode uninterpreted data
    auto const misc_data = input->uninterpreted_frame_data();
    if( misc_data )
    {
      output->add_uninterpreted_data( *misc_data );
    }

    // Transcode image
    if( cmd_args.count( "copy-video" ) )
    {
      auto const image = input->raw_frame_image();
      if( !image )
      {
        std::cerr << "No raw image found for frame " << timestamp.get_frame()
                  << "." << std::endl;
        exit(-1);
      }
      output->add_image( *image );
    }
    else
    {
      output->add_image( input->frame_image(), timestamp );
    }
  }

  // Clean up
  input->close();
  output->close();

  return EXIT_SUCCESS;
}

} // namespace core

} // namespace arrows

} // namespace kwiver
