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

/** \file
  * \brief C Interface to \p vital::config_manager class
 */
#ifndef VITAL_C_PLUGIN_MANAGER_H
#define VITAL_C_PLUGIN_MANAGER_H

#include <vital/bindings/c/common.h>
#include <vital/bindings/c/vital_c_export.h>
#include <vital/bindings/c/error_handle.h>

#include <vital/plugin_loader/plugin_manager.h>

#ifdef __cplusplus
extern "C"
{
#endif

// VITAL_C_EXPORT
// plugin_manager plugin_manager_instance();

VITAL_C_EXPORT
void plugin_manager_load_all_plugins();

VITAL_C_EXPORT
void plugin_manager_load_plugins( size_t count, char** path_list );

VITAL_C_EXPORT
void plugin_manager_add_search_path_list( size_t count, char** path_list );

VITAL_C_EXPORT
void plugin_manager_add_search_path( char const* path );

VITAL_C_EXPORT
plugin_factory_handle_t plugin_manager_add_factory( plugin_factory* fact );

// VITAL_C_EXPORT
// plugin_factory_vector_t const& plugin_manager_get_factories( char const* type_name );

VITAL_C_EXPORT
void plugin_manager_reload_plugins();

VITAL_C_EXPORT
bool plugin_manager_is_module_loaded( char const* name);

VITAL_C_EXPORT
void plugin_manager_mark_module_as_loaded( char const* name);

VITAL_C_EXPORT
void plugin_manager_add_path_from_environment( char const* env_name );

#ifdef __cplusplus
}
#endif

#endif /* VITAL_C_PLUGIN_MANAGER_H */
