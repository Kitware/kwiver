// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/applets/kwiver_applet.h>
#include <vital/applets/applet_context.h>

#include <vital/applets/applet_registrar.h>
#include <vital/exceptions/base.h>
#include <vital/plugin_loader/plugin_factory.h>
#include <vital/plugin_loader/plugin_manager_internal.h>
#include <vital/util/get_paths.h>
#include <vital/util/tokenize.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>

using applet_context_t = std::shared_ptr< kwiver::tools::applet_context >;
namespace kv = kwiver::vital;

class command_node;
using command_node_sptr = std::shared_ptr<command_node>;
bool compare_node( command_node_sptr const& n1, command_node_sptr const& n2 );

// -------------------------------------------------------------------
class command_node
{
public:
  command_node(std::string const& name)
    : m_name( name ),
      m_is_command( false )
  { }

  virtual ~command_node() = default;

  void
  add_factory( kv::plugin_factory_handle_t fact )
  {
    m_factory = fact;
    m_is_command = true;
  }

  bool
  is_command() const { return m_is_command; }

  command_node_sptr
  find( std::string const& name )
  {
    for ( auto& n : m_nodes )
    {
      if ( name == n->m_name )
      {
        return n;
      }
    } // end for
    return nullptr;
  }

  void
  add( command_node_sptr n )
  {
    m_nodes.push_back(n);

    // sort vector
    std::sort( m_nodes.begin(), m_nodes.end(), compare_node );
    return;
  }

  void
  print_subtree_help( std::string const& pfx = "")
  {
    for ( auto const& n : m_nodes )
    {
      n->print_subtree_help( pfx + " " + this->m_name );
    }

    if( this->is_command() )
    {
      std::cout << pfx << " " << this->m_name << " " << std::endl;
      return;
    }
  }

  void
  dump_tree(std::string const& pfx = "") const
  {
    std::cout << pfx << "Name: " << m_name << "   is_command: " << is_command() << "\n";
    for ( auto const& n : m_nodes )
    {
      n->dump_tree( pfx + "    " );
    }
  }

  std::string m_name;
  bool m_is_command { false };
  std::vector<command_node_sptr> m_nodes;
  kv::plugin_factory_handle_t m_factory;
};

// --------------------------------------------------------------------
bool compare_node( command_node_sptr const& n1, command_node_sptr const& n2 )
{
  return (n1->m_name < n2->m_name);
}

// ============================================================================
/**
 * This class processes the incoming list of command line options.
 * They are separated into oprtions for the tool runner and options
 * for the applet.
 */
class command_line_parser
{
public:
  command_line_parser( int p_argc, char *** p_argv )
  {
    int state(0);

    // Parse the command line
    // Command line format:
    // arg0 [runner-flags] <applet path> [applet-args]

    // The first applet args is the program name.
    m_applet_args.push_back( "kwiver" );

    // Separate args into groups
    for (int i = 1; i < p_argc; ++i )
    {
      if (state == 0)
      {
        // look for an option flag
        if ( (*p_argv)[i][0] == '-' )
        {
          m_runner_args.push_back( (*p_argv)[i] );
        }
        else
        {
          // found applet name component
          //+ Should just change state and re-evaluate token
          m_applet_name.push_back( std::string( (*p_argv)[i] ));
          state = 1;
        }
      }
      else if (state == 1)
      {
       if ( (*p_argv)[i][0] == '-' )
        {
          // Collecting applet parameters
          m_applet_args.push_back( (*p_argv)[i] );
          state = 2;
        }
       else
       {
          m_applet_name.push_back( std::string( (*p_argv)[i] ));
       }
      }
      else if (state == 2)
      {
          // Collecting applet parameters
          m_applet_args.push_back( (*p_argv)[i] );
      }
    } // end for

  }

  // ----------------------
  // tool runner arguments.
  std::string m_output_file; // empty is no file specified

  std::vector<std::string> m_runner_args;
  std::vector<std::string> m_applet_args;

  // Applet name may contain args but we can't tell before running
  // these words against the command tree.
  std::vector<std::string> m_applet_name;
};

// --------------------------------------------------------------------
// This function parses all applets and builds the command
// tree, which is returned
command_node_sptr
build_command_tree( kwiver::vital::plugin_manager& vpm )
{
  command_node_sptr root = std::make_shared<command_node>( "Root" );

  // Get list of factories for implementations of the applet
  const auto fact_list = vpm.get_factories( typeid( kwiver::tools::kwiver_applet ).name() );

  for( auto fact : fact_list )
  {
    std::string buf = "-- Not Set --";
    fact->get_attribute( kv::plugin_factory::PLUGIN_NAME, buf );

    // split string on ';' for multiple paths
    std::vector<std::string> paths;
    tokenize( buf, paths, ";", kv::TokenizeTrimEmpty );

    for ( auto const& w : paths )
    {
      auto current_node = root;
      // split each path string on ':'
      std::vector<std::string> words;
      tokenize( w, words, ":", kv::TokenizeTrimEmpty );

      for ( auto const& c : words )
      {
        command_node_sptr n = current_node->find( c );
        if ( !n )
        {
          // node doesn't exist, need to add it
          n =  std::make_shared< command_node >( c );
          current_node->add( n );
        }
        else if ( n && n->is_command() )
        {
          // error found a terminal node
          std::cerr << "Could not register applet. Command \"";
          for ( auto const& ew : words )
          {
            std::cerr << ew;
            if ( ew == c )
            {
              break;
            }
            std::cerr << " ";

          }
          std::cerr << "\" already exists. Defined by ";
          auto nn = current_node->find(c);
          auto f = nn->m_factory;
          std::string plugin_file;
          f->get_attribute( kv::plugin_factory::PLUGIN_FILE_NAME, plugin_file );
          std::cerr << plugin_file << std::endl;
          exit( -1 );
        }

        current_node = n;
      } // end for

      // Add factory to the last node.
      current_node->add_factory( fact );
    }
  } // end for

  return root;
}

// ----------------------------------------------------------------------------
/**
 * Generate list of all applets that have been discovered.
 */
void tool_runner_usage( VITAL_UNUSED applet_context_t ctxt,
                        kwiver::vital::plugin_manager& vpm )
{
  // display help message
  std::cout << "Usage: kwiver <tool>  [args]" << std::endl
            << "<tool> can be one of the following:" << std::endl
            << "help - prints this message." << std::endl
            << "Available tools are listed below:" << std::endl;

  // Get list of factories for implementations of the applet
  const auto fact_list = vpm.get_factories( typeid( kwiver::tools::kwiver_applet ).name() );

  // Loop over all factories in the list and display name and description
  using help_pair = std::pair< std::string, std::string >;
  std::vector< help_pair > help_text;
  size_t tab_stop(0);

  for( auto fact : fact_list )
  {
    std::string buf = "-- Not Set --";
    fact->get_attribute( kwiver::vital::plugin_factory::PLUGIN_NAME, buf );
    // need to format the name since the command words are separated by ':'
    std::vector<std::string> paths;
    tokenize( buf, paths, ";", kv::TokenizeTrimEmpty );

    std::string descr = "-- Not Set --";
    fact->get_attribute( kwiver::vital::plugin_factory::PLUGIN_DESCRIPTION, descr );

    // All we want is the first line of the description.
    size_t pos = descr.find_first_of('\n');
    if ( pos != 0 )
    {
      // Take all but the ending newline
      descr = descr.substr( 0, pos );
    }

    // make help entry for all name variants
    for ( auto name : paths )
    {
      std::replace( name.begin(), name.end(), ':', ' ');
      help_text.push_back( help_pair({ name, descr }) );
      tab_stop = std::max( tab_stop, name.size() );
    }
  } // end for

  // add some space after the longest applet name
  tab_stop += 2;

  // sort the applet names
  sort( help_text.begin(), help_text.end() );

  for ( auto const& elem : help_text )
  {
    const size_t filler = tab_stop - elem.first.size();
    std::cout << elem.first << std::string( filler, ' ') << elem.second << std::endl;
  }
}

// ============================================================================
int main(int argc, char *argv[])
{
command_node_sptr cmd_root;

  //
  // Global shared context
  // Allocated on the stack so it will automatically clean up
  //
  applet_context_t tool_context = std::make_shared< kwiver::tools::applet_context >();

  kwiver::vital::plugin_manager_internal& vpm = kwiver::vital::plugin_manager_internal::instance();

  const std::string exec_path = kwiver::vital::get_executable_path();
  vpm.add_search_path(exec_path + "/../lib/kwiver/plugins");

  vpm.load_all_plugins();

  // initialize the global context
  tool_context->m_wtb.set_indent_string( "      " );
  cmd_root = build_command_tree( vpm );
  command_line_parser options( argc, &argv );

  // If there is only a 'help' as an argument
  if ( (argc == 1) || (options.m_applet_name[0] == "help") )
  {
    tool_runner_usage( tool_context, vpm );
    return 0;
  } // end help code

  // ----------------------------------------------------------------------------
  try
  {
    // Create applet based on the name provided
    auto cn = cmd_root;
    size_t ci;
    for ( ci = 0; ci < options.m_applet_name.size(); ci++)
    {
      auto n = cn->find( options.m_applet_name[ci] );
      if ( ! n )
      {
        break;
      }
      cn = n;
    } // end for

    // If there are no matches or we run out of command words
    // before we reach a terminal command node.
    if ( ci == 0 && !cn->is_command() )
    {
      // error - no command match at all. Give full help
      std::cerr << "Command not found.\n\n";
      tool_runner_usage( tool_context, vpm );
      exit(-1);
    }
    else if ( ci > 0 && !cn->is_command() )
    {
      // Partial command match - just subtree help
      std::cerr << "Command not found. Related commands are as follows:\n";
      cn->print_subtree_help();
      exit(-1);
    }

    // Insert name words[ci+1] [size-1] at start of options.m_applet_args
    for( size_t ai = ci; ai < options.m_applet_name.size(); ai++)
    {
      options.m_applet_args.insert(
        options.m_applet_args.begin()+1, // skip first "kwiver" element
        options.m_applet_name[ai] );
    } // end for

    kwiver::tools::kwiver_applet_sptr applet(
      cn->m_factory->create_object<kwiver::tools::kwiver_applet>() );

    tool_context->m_applet_name = options.m_applet_name;
    tool_context->m_argv = options.m_applet_args; // save a copy of the args

    // Pass the context to the applet. This is done as a separate call
    // because the default factory for applets does not take any
    // parameters.
    applet->initialize( tool_context.get() );

    // Call the applet so it can add the commands that it is looking
    // for.
    applet->add_command_options();

    int local_argc = 0;
    char** local_argv = 0;
    std::vector<char *> argv_vect;

    // There are some cases where the applet wants to do its own
    // command line parsing (e.g. QT apps). If this flag is not set,
    // then we will parse our standard arg set
    if ( ! tool_context->m_skip_command_args_parsing )
    {
      // Convert args list back to argv style. :-(
      // This is needed for the command options support package.
      argv_vect.resize( options.m_applet_args.size()+1, nullptr );
      for (std::size_t i = 0; i != options.m_applet_args.size(); ++i)
      {
        argv_vect[i] = &options.m_applet_args[i][0];
      }
    }
    else
    {
      argv_vect.resize( 2, nullptr );
      argv_vect[0] = &options.m_applet_args[0][0];
    }

    local_argc = argv_vect.size()-1;
    local_argv = &argv_vect[0];

    // The parse result has to be created locally due to class design.
    // No default CTOR, copy CTOR or copy operation so we just have to use the local copy.
    cxxopts::ParseResult local_result = applet->m_cmd_options->parse( local_argc, local_argv );

    // Make results available in the context,
    tool_context->m_result = &local_result; // in this case the address of a stack variable is o.k.

    // Run the specified tool
    return applet->run();
  }
  catch ( cxxopts::OptionException& e)
  {
    std::cerr << "Command argument error: " << e.what() << std::endl;
    exit( -1 );
  }
  catch ( kwiver::vital::plugin_factory_not_found& )
  {
    std::cerr << "Tool \"" << argv[1] << "\" not found. Type \""
              << argv[0] << " help\" to list available tools." << std::endl;

    exit(-1);
  }
  catch ( kv::vital_exception& e )
  {
    std::cerr << "Caught unhandled kwiver::vital::vital_exception: " << e.what() << std::endl;
    exit( -1 );
  }
  catch ( std::exception& e )
  {
    std::cerr << "Caught unhandled std::exception: " << e.what() << std::endl;
    exit( -1 );
  }
  catch ( ... )
  {
    std::cerr << "Caught unhandled exception" << std::endl;
    exit( -1 );
  }

  return 0;
}
