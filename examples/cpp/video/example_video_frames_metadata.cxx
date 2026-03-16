// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "vital/algo/algorithm.txx"
#include "vital/algo/video_input.h"
#include "vital/config/config_block_io.h"
#include "vital/plugin_management/plugin_manager.h"
#include "vital/types/metadata.h"
#include "vital/types/metadata_traits.h"
#include "vital/types/timestamp.h"
#include "vital/version.h"

#include "kwiversys/CommandLineArguments.hxx"
#include "kwiversys/SystemTools.hxx"

using namespace kwiver::vital;

void
print_metadata( const metadata_vector& metadata, bool detail )
{
  // This function prints elements of the provided metadata on the current
  // frame.
  for( const auto& meta : metadata )
  {
    std::cout << "\n\n---------------- Metadata from: " << meta->timestamp() <<
      std::endl;
    if( detail )
    {
      // Iterating over all the metadata items
      for( const auto& item_pair : *meta )
      {
        const std::shared_ptr< metadata_item >& item = item_pair.second;

        // Getting the name
        const std::string& name = item->name();
        // Getting the tag so we can retrieve the traits
        vital_metadata_tag tag = item->tag();
        const metadata_tag_traits& trait = tag_traits_by_tag( tag );
        const std::string& description = trait.description();

        std::cout << "Metadata item: " << name << std::endl << description <<
          std::endl
                  << "Data: < " << item->type().name() << " >: " <<
          item->as_string() << std::endl;
      }
    }
  }
}

void
example_video_frames_metadata(
  const std::string& video_file, const std::string& config_file, bool detail )
{
  // This example loads a video provided as input by the user, with an optional
  // configuration file.
  // Frames are then iterated upon and metadata is extracted according
  // to the configuration.
  // The metadata is then printed in the terminal with a verbosity level
  // dependent
  // on the input variable detail

  // Loading all the plugins
  plugin_manager::instance().load_all_plugins();

  // Instantiating the video reader
  algo::video_input_sptr video_reader;

  // We load a default configuration file
  std::string dump_klv_config = std::string( BINARY_ROOT_DIR ) +
                                "/share/kwiver/" + KWIVER_VERSION +
                                "/config/applets/dump_klv.conf";

  // Reading the config file
  config_block_sptr config = read_config_file( dump_klv_config );

  // If we have a configuration file provided by the user, we merge it with the
  // one we already have.
  if( config_file != "" )
  {
    config_block_sptr extra_config = read_config_file( config_file );
    config->merge_config( extra_config );
  }

  // Setting the configuration in the video reader.
  set_nested_algo_configuration< algo::video_input >(
    "video_reader", config,
    video_reader );

  // Opening the provided video
  video_reader->open( video_file );

  while( video_reader->next_frame() )
  {
    // Extracting the metadata at the current frame
    metadata_vector metadata = video_reader->frame_metadata();
    print_metadata( metadata, detail );
  }
}

int
main( int argc, char* argv[] )
{
  kwiversys::CommandLineArguments arg;
  arg.Initialize( argc, argv );

  std::string config_file;
  bool detail;
  std::string video_file;

  arg.AddArgument(
    "-c", kwiversys::CommandLineArguments::SPACE_ARGUMENT, &config_file,
    "Configuration file." );
  arg.AddArgument(
    "-d", kwiversys::CommandLineArguments::NO_ARGUMENT, &detail,
    "Display a detailed description of the metadat" );
  arg.AddArgument(
    "-i", kwiversys::CommandLineArguments::SPACE_ARGUMENT, &video_file,
    "Video input file" );

  if( !arg.Parse() || video_file.empty() )
  {
    std::cerr << "Missing video file name." << std::endl;
    return EXIT_FAILURE;
  }

  example_video_frames_metadata( video_file, config_file, detail );

  return EXIT_SUCCESS;
}
