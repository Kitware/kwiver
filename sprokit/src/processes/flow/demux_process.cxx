/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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

/**
 * \file demux_process.cxx
 *
 * \brief Implementation of the demux process.
 */

#include "demux_process.h"

#include <vital/vital_foreach.h>
#include <vital/util/tokenize.h>

#include <sprokit/pipeline/datum.h>
#include <sprokit/pipeline/edge.h>
#include <sprokit/pipeline/process_exception.h>
#include <sprokit/pipeline/stamp.h>

#include <map>
#include <string>
#include <algorithm>

namespace sprokit {

// \todo Change terminology from tag to group or bundle. Tag is not explicit enough.
//
// Design issues:
//
// Use in/group/item approach where the items in a group are ordered in
// an ASCII-betical order.
//
// Need multiple termination semantics. Terminate group when one
// port is complete.  Terminate when all inputs are complete.
// Termination policies are "any" and "all"
//
// "any" will cause the group to complete if any of the inputs completes.
//
// "all" will cause the group to complete when all inputs complete.
//

/**
 * \class demux_process
 *
 * \brief A process for collating input data from multiple input edges.
 *
 * \process Demux incoming data into a single stream.  A collation
 * operation reads input from a set of input ports and serializes that
 * data to a single output port. This collation process can handle
 * multiple collation operations. Each set of collation ports is
 * identified by a unique \b group name.
 *
 * \iports
 *
 *
 * \oports
 *
 * \oport{res/\portvar{tag}} The demuxd result \portvar{tag}.
 *
 * \reqs
 *
 * \req Each \portvar{tag} must have at least two inputs to demux.
 * \req Each output port \port{res/\portvar{tag}} must be connected.
 *
 * This process automatically makes the input and output types for
 * each \b tag the same based on the type of the port that is first
 * connected.
 *
 * \note
 * It is not immediately apparent how the input ports become sorted in
 * ASCII-betical order on "item" order.
 *
 * \code
 process demux :: demux_process

 # -- Connect demux set "input1"
 connect foo_1.out       to  demux.coll/input1/A
 connect foo_2.out       to  demux.coll/input1/B

 connect demux.res/input1  to bar.input # connect output

 # -- Connect demux set "input2"
 connect foo_1.out       to  demux.coll/input2/A
 connect foo_2.out       to  demux.coll/input2/B
 connect foo_3.out       to  demux.coll/input2/C

 connect demux.res/input2  to bar.other # connect output

 * \endcode
 *
 * \todo Add configuration to allow forcing a number of inputs for a result.
 * \todo Add configuration to allow same number of sources for all results.
 *
 * \ingroup process_flow
 */

class demux_process::priv
{
public:
  priv();
  ~priv();

  enum term_policy_t { skip, policy_all, policy_any };
  typedef port_t group_t;

  // This class stores info for each group.
  class group_info
  {
  public:
    group_info();
    ~group_info();

    ports_t ports; // vector of port names
    ports_t::iterator cur_port;
  };
  typedef std::map< group_t, group_info > group_data_t;

  group_data_t group_data; // group table

  group_t group_for_port( port_t const& port ) const;

  static port_t const res_sep;
  static port_t const port_res_prefix;
  static port_t const port_in_prefix;

  term_policy_t m_config_term_policy;
};

process::port_t const demux_process::priv::res_sep = port_t( "/" );
process::port_t const demux_process::priv::port_res_prefix = port_t( "res" ) + res_sep;
process::port_t const demux_process::priv::port_in_prefix = port_t( "in" ) + res_sep;

/**
 * \internal
 *
 * Ports on the \ref distribute_process are broken down as follows:
 *
 *   \portvar{type}/\portvar{group}[/\portvar{item}]
 *
 * The port name is broken down as follows:
 *
 * <dl>
 * \term{\portvar{type}}
 *   \termdef{The type of the port. This must be one of \type{res},
 *   or \type{coll}.}
 * \term{\portvar{group}}
 *   \termdef{The name of the stream the port is associated with.}
 * \term{\portvar{item}}
 *   \termdef{Only required for \type{coll}-type ports. Data from the same
 *   \portvar{group} stream from its \type{res} port is collected in sorted order
 *   over all of the \type{coll} ports.}
 * </dl>
 *
 * The available port types are:
 *
 * <dl>
 * \term{\type{res}}
 *   \termdef{This port for the given group is where the data for a stream leaves
 *   the process.
 * \term{\type{coll}}
 *   \termdef{These ports for a given \portvar{group} receive data from a set of
 *   sources, likely made by the \ref distribute_process. Data is collected in
 *   sorted ordef of the \type{item} name and sent out the \type{res} port for
 *   the \portvar{group}.}
 * </dl>
 */

demux_process
::demux_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
  d( new priv )
{
  // This process manages its own inputs.
  this->set_data_checking_level( check_none );

  declare_configuration_key( "termination_policy", "any",
                             "Termination policy specifies how a data group is handled when the inputs complete. "
                             "Valid values are \"any\" and \"all\". "
                             "When \"any\" is specified, the output port for the group will complete when any of "
                             "the inputs completes and the remaining active inputs will no longer be polled for data. "
                             "When \"all\" is specified, the output port for the group will complete when all of "
                             "the inputs are complete. "
    );

}


demux_process
::~demux_process()
{
}


// ----------------------------------------------------------------
void
demux_process
::_configure()
{
  // Examine the configuration
  const std::string value = config_value< std::string >( "termination_policy" );

  if ( value == "any" )
  {
    d->m_config_term_policy = demux_process::priv::term_policy_t::policy_any;
  }
  else if ( value == "all" )
  {
    d->m_config_term_policy = demux_process::priv::term_policy_t::policy_all;
  }
  else
  {
    std::string const reason = "Invalid option specified for termination_policy: " + value;
    throw invalid_configuration_exception( name(), reason );
  }
}


// ------------------------------------------------------------------
// Post connection processing
void
demux_process
::_init()
{
  VITAL_FOREACH( priv::group_data_t::value_type & group_data, d->group_data )
  {
    priv::group_t const& group = group_data.first;
    priv::group_info& info = group_data.second;
    ports_t const& ports = info.ports; // list of port names

    if ( ports.size() < 2 )
    {
      std::string const reason = "There must be at least two ports to demux "
                                 "to for the \"" + group + "\" result data";

      throw invalid_configuration_exception( name(), reason );
    }

    // Now here's some port frequency magic
    frequency_component_t const ratio = ports.size();
    port_frequency_t const freq = port_frequency_t( 1, ratio );

    // Set port frequency for all input ports.
    VITAL_FOREACH( port_t const & port, ports )
    {
      set_input_port_frequency( port, freq );
    }

    // Set iterator to start of list.
    info.cur_port = info.ports.begin();
  }

}


// ------------------------------------------------------------------
void
demux_process
::_reset()
{
  VITAL_FOREACH( priv::group_data_t::value_type const & group_data, d->group_data )
  {
    priv::group_t const& group = group_data.first;
    port_t const output_port = priv::port_res_prefix + group;
    priv::group_info const& info = group_data.second;
    ports_t const& ports = info.ports;

    VITAL_FOREACH( port_t const & port, ports )
    {
      remove_input_port( port );
    }

    remove_output_port( output_port );
  }

  d->group_data.clear();
}


// ------------------------------------------------------------------
void
demux_process
::_step()
{
  ports_t complete_ports;

  // Loop over all groups (input groups)
  VITAL_FOREACH( priv::group_data_t::value_type & group_data, d->group_data )
  {
    priv::group_t const& group = group_data.first;
    port_t const output_port = priv::port_res_prefix + group;
    priv::group_info& info = group_data.second;

    // There is real data on the input ports. Grab data from the
    // current input port and push to the output.
    edge_datum_t const input_edat = grab_from_port( *info.cur_port );

    // check for complete on input port
    datum_t const& input_dat = input_edat.datum;
    datum::type_t const input_type = input_dat->type();

    // Test to see if complete.
    // If the upstream process is done, then mark this group as done.
    if ( input_type == datum::complete )
    {
      // check with termination policy.
      switch ( d->m_config_term_policy )
      {
      case demux_process::priv::term_policy_t::policy_any:
        // Flush this set of inputs
        VITAL_FOREACH (port_t const& port, info.ports)
        {
          (void)grab_from_port(port);
        }

        // echo the input control message to the output port
        push_to_port(output_port, input_edat);

        complete_ports.push_back( group );
        break;

      case demux_process::priv::term_policy_t::policy_all:
      {
        // remove this port only from the "group_data"
        info.cur_port = info.ports.erase( info.cur_port ); // updates iterator
        // need to check for wrapping past end.
        if ( info.cur_port == info.ports.end() )
        {
          info.cur_port = info.ports.begin();
        }

        // If there are no more input ports in this group.
        if ( info.ports.empty() )
        {
          complete_ports.push_back( group );

          // echo the input control message to the output port
          push_to_port(output_port, input_edat);
        }
      }
      break;

      default:
        throw invalid_configuration_exception( name(), "Invalid option specified for termination_policy." );

      } // end switch

      continue;
    } // end datum::complete

    // Send the input to the output port.
    push_datum_to_port( output_port, input_dat );

    // Advance to next port in the group, and wrap at the end.
    ++info.cur_port;
    if ( info.cur_port == info.ports.end() )
    {
      info.cur_port = info.ports.begin();
    }
  } // end foreach

  // Process all ports/groups that have completed. When a status port
  // reports complete on a group, that group is erased from the local
  // map. When that map is empty, then we are all done and can complete.
  VITAL_FOREACH( port_t const & port, complete_ports )
  {
    d->group_data.erase( port );
  }

  if ( d->group_data.empty() )
  {
    mark_process_as_complete();
  }

} // demux_process::_step


// ------------------------------------------------------------------
process::properties_t
demux_process
::_properties() const
{
  properties_t consts = process::_properties();

  consts.insert( property_unsync_input );

  return consts;
}


// ------------------------------------------------------------------
// Intercept input port connection so we can create the requested port
process::port_info_t
demux_process
::_input_port_info( port_t const& port )
{
  //+ need to accept connections from "in/<group>/<item>" and create
  // output "res/<group>" output port the first time.

  // Extract GROUP sub-string from port name
  ports_t components;
  kwiver::vital::tokenize( port, components, priv::res_sep );

  // Results are:
  // components[0] = "in"
  // components[0] = "group"
  // components[0] = "item"

  // Port name must start with "in/"
  if ( port.compare( 0, priv::port_in_prefix.size(), priv::port_in_prefix ) && ( components.size() == 3 ))
  {
    const priv::group_t group = components[1];

    // If GROUP does not exist, then this is the first port in the group
    if ( 0 == d->group_data.count( group ) )
    {
      // This is the first port of this group
      priv::group_info info;
      d->group_data[group] = info;

      port_flags_t required;
      required.insert( flag_required );

      // Create output port "res/group"
      declare_output_port(
        priv::port_res_prefix + group,
        type_flow_dependent + group, // note the group magic on port type
        required,
        port_description_t( "The output port for " + group + "." ) );
    }

    // Get entry based on the group string
    priv::group_info& info = d->group_data[group];

    // Add this port to the info list for this group
    info.ports.push_back( port );

    port_flags_t required;
    required.insert( flag_required );

    // Open an input port for the name
    declare_input_port(
      port,
      type_flow_dependent + group, // note the group magic on port type
      required,
      port_description_t( "An input for the " + group + " data." ) );
  }
  return process::_input_port_info( port );
} // demux_process::_input_port_info


// ------------------------------------------------------------------
demux_process::priv
::priv()
  : group_data()
  , m_config_term_policy( skip )
{
}


demux_process::priv
::~priv()
{
}


// ------------------------------------------------------------------
/*
 * @brief Find group name that corresponds to the port name.
 *
 * This method looks through the list of current groups to see if the
 * supplied port is in that table.
 *
 * @param port Name of the port
 *
 * @return Group name
 */
demux_process::priv::group_t
demux_process::priv
::group_for_port( port_t const& port ) const
{
  // Does this port start with "in/"
  if ( port.compare( 0, priv::port_in_prefix.size(), priv::port_in_prefix ) )
  {
    // Get the part of the port name after the prefix
    // This could be "group/item"
    port_t const no_prefix = port.substr( priv::port_in_prefix.size() );

    // loop over all groups seen so far
    VITAL_FOREACH( priv::group_data_t::value_type const & data, group_data )
    {
      group_t const& group = data.first; // group string
      port_t const group_prefix = group + priv::res_sep;

      // If the port name without the prefix is "group/*" then return
      // base group string
      if ( no_prefix.compare( 0, group_prefix.size(), group_prefix ) )
      {
        return group;
      }
    }
  }

  return group_t();
}


// ------------------------------------------------------------------
demux_process::priv::group_info
::group_info()
  : ports(),
  cur_port()
{
}


demux_process::priv::group_info
::~group_info()
{
}

} // end namespace
