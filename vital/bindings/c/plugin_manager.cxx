/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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

#include "plugin_manager.h"

#include <vital/plugin_loader/plugin_manager.h>


// ------------------------------------------------------------------
VITAL_C_EXPORT
plugin_manager plugin_manager_instance()
{
  return &kwiver::vital::plugin_manager::instance();
}


// ------------------------------------------------------------------
VITAL_C_EXPORT
void plugin_manager_load_all_plugins()
{
  kwiver::vital::plugin_manager::instance().load_plugins();
}


// ------------------------------------------------------------------
VITAL_C_EXPORT
void plugin_manager_load_plugins( size_t count, char** path_list )
{
  kwiver::vital::path_list_t list;
  for (size_t i = 0; i < count; ++i)
  {
    list.push_back( path_list[i] );
  }

  kwiver::vital::plugin_manager::instance().load_plugins( list );
}


// ------------------------------------------------------------------
VITAL_C_EXPORT
void plugin_manager_add_search_path( size_t count, char** path_list )
{
  kwiver::vital::path_list_t list;
  for (size_t i = 0; i < count; ++i)
  {
    list.push_back( path_list[i] );
  }

  kwiver::vital::plugin_manager::instance().add_search_path( list );
}


// ------------------------------------------------------------------
VITAL_C_EXPORT
plugin_factory_handle_t plugin_manager_add_factory( plugin_factory* fact )
{
  kwiver::vital::plugin_manager::instance().add_factory( fact );
}


// ------------------------------------------------------------------
// VITAL_C_EXPORT
// plugin_factory_vector_t const& plugin_manager_get_factories( char const* type_name )
// {
//   //* problem here
// }


// ------------------------------------------------------------------
VITAL_C_EXPORT
void plugin_manager_reload_plugins()
{
  kwiver::vital::plugin_manager::instance().reload_plugins();
}


// ------------------------------------------------------------------
VITAL_C_EXPORT
bool plugin_manager_is_module_loaded( char const* name)
{
  std::string str_name( name );
  return kwiver::vital::plugin_manager::instance().is_module_loaded( str_name );
}


// ------------------------------------------------------------------
VITAL_C_EXPORT
void plugin_manager_mark_module_as_loaded( char const* name )
{
  std::string str_name( name );
  kwiver::vital::plugin_manager::instance().mark_module_as_loaded( str_name );
}

// ------------------------------------------------------------------
VITAL_C_EXPORT
void plugin_manager_add_path_from_environment( char const* env_var)
{
  std::string str_name( env_var );
  kwiver::vital::plugin_manager::instance().add_path_from_environment( str_name );
}
