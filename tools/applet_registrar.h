/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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

#ifndef KWIVER_TOOLS_KWIVER_APPLET_REGISTER_H
#define KWIVER_TOOLS_KWIVER_APPLET_REGISTER_H

#include <vital/plugin_loader/plugin_registrar.h>
#include <tools/kwiver_applet.h>

namespace kwiver {

class applet_registrar
  : public plugin_registrar
{
public:
  applet_registrar( kwiver::vital::plugin_loader& vpl,
                    const std::string& mod_name )
    : plugin_registrar( vpl, mod_name )
  {
  }


  // ----------------------------------------------------------------------------
  template <typename tool_t>
  kwiver::vital::plugin_factory_handle_t register_tool()
  {
    using kvpf = kwiver::vital::plugin_factory;

    auto fact = plugin_loader().
      add_factory( new kwiver::vital::plugin_factory_0< tool_t >(
                     typeid( kwiver::tools::kwiver_applet ).name() ) );

    fact->add_attribute( kvpf::PLUGIN_NAME,      tool_t::_plugin_name )
      .add_attribute( kvpf::PLUGIN_DESCRIPTION,  tool_t::_plugin_description )
      .add_attribute( kvpf::PLUGIN_VERSION,      tool_t::_plugin_version )
      .add_attribute( kvpf::PLUGIN_MODULE_NAME,  this->module_name() )
      .add_attribute( kvpf::PLUGIN_ORGANIZATION, this->organization() )
      .add_attribute( kvpf::PLUGIN_CATEGORY,     "kwiver-applet" )
      ;

    return fact;
  }
};

} // end namespace

#endif /* KWIVER_TOOLS_KWIVER_APPLET_REGISTER_H */
