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

#ifndef KWIVER_ARROWS_CORE_TOOLS_RENDER_MESH_H
#define KWIVER_ARROWS_CORE_TOOLS_RENDER_MESH_H

#include <tools/kwiver_applet.h>

#include <arrows/core/applets/kwiver_arrows_core_applets_export.h>

#include <string>
#include <vector>

namespace kwiver {
namespace arrows {
namespace core {

class KWIVER_ARROWS_CORE_APPLETS_EXPORT render_mesh
  : public kwiver::tools::kwiver_applet
{
public:
  render_mesh(){}
  virtual ~render_mesh() = default;

  static constexpr char const* name = "render-mesh";
  static constexpr char const* description =
    "Render a depth or height map from a mesh.\n\n"
    "This tool reads in a mesh file and a camera and renders "
    "various images such as depth map or height map.";


  virtual int run( const std::vector<std::string>& argv );
  virtual void usage( std::ostream& outstream ) const;

protected:

private:

}; // end of class

} } } // end namespace

#endif /* KWIVER_ARROWS_CORE_TOOLS_RENDER_MESH_H */
