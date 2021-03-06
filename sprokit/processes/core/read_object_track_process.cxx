// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation for read_object_track_set process
 */

#include "read_object_track_process.h"

#include <vital/vital_types.h>
#include <vital/exceptions.h>
#include <vital/algo/read_object_track_set.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace algo = kwiver::vital::algo;

namespace kwiver {

create_algorithm_name_config_trait( reader );
create_config_trait( file_name, std::string, "",
  "Name of the track descriptor set file to read." );

//--------------------------------------------------------------------------------
// Private implementation class
class read_object_track_process::priv
{
public:
  priv();
  ~priv();

  // Configuration values
  std::string m_file_name;

  algo::read_object_track_set_sptr m_reader;
}; // end priv class

// ===============================================================================

read_object_track_process
::read_object_track_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new read_object_track_process::priv )
{
  make_ports();
  make_config();
}

read_object_track_process
::~read_object_track_process()
{
}

// -------------------------------------------------------------------------------
void read_object_track_process
::_configure()
{
  // Get process config entries
  d->m_file_name = config_value_using_trait( file_name );

  if( d->m_file_name.empty() )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(),
      "Required file name not specified." );
  }

  // Get algo config entries
  kwiver::vital::config_block_sptr algo_config = get_config(); // config for process

  // validate configuration
  if( ! algo::read_object_track_set::check_nested_algo_configuration_using_trait(
          reader, algo_config ) )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(),
                 "Configuration check failed." );
  }

  // instantiate image reader and converter based on config type
  algo::read_object_track_set::set_nested_algo_configuration_using_trait(
    reader,
    algo_config,
    d->m_reader );

  if( ! d->m_reader )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(),
                 "Unable to create reader." );
  }
}

// -------------------------------------------------------------------------------
void read_object_track_process
::_init()
{
  d->m_reader->open( d->m_file_name ); // throws
}

// -------------------------------------------------------------------------------
void read_object_track_process
::_step()
{
  std::string image_name;
  kwiver::vital::object_track_set_sptr set;

  if( d->m_reader->read_set( set ) )
  {
    push_to_port_using_trait( object_track_set, set );
  }
  else
  {
    LOG_DEBUG( logger(), "End of input reached, process terminating" );

    // indicate done
    mark_process_as_complete();
    const sprokit::datum_t dat= sprokit::datum::complete_datum();

    push_datum_to_port_using_trait( object_track_set, dat );
  }
}

// -------------------------------------------------------------------------------
void read_object_track_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;

  declare_output_port_using_trait( object_track_set, optional );
}

// -------------------------------------------------------------------------------
void read_object_track_process
::make_config()
{
  declare_config_using_trait( file_name );
  declare_config_using_trait( reader );
}

// ===============================================================================
read_object_track_process::priv
::priv()
{
}

read_object_track_process::priv
::~priv()
{
}

} // end namespace
