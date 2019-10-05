/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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

#ifndef INCL_FILE_FORMAT_KPF_ACTIVITY_H
#define INCL_FILE_FORMAT_KPF_ACTIVITY_H

#include <vital/vital_config.h>
#include <track_oracle/file_formats/track_kw18/track_kw18_export.h>

#include <track_oracle/file_formats/file_format_base.h>
#include <track_oracle/file_formats/track_kpf_activity/track_kpf_activity.h>

namespace kwiver {
namespace track_oracle {

class TRACK_KPF_ACTIVITY_EXPORT file_format_kpf_activity: public file_format_base
{
public:
  file_format_kpf_activity(): file_format_base( TF_KPF_ACT, "KPF activities" )
  {
    this->globs.push_back( "*.activities.yml" );
  }
  virtual ~file_format_kpf_activity() {}

  virtual int supported_operations() const { return FF_READ | FF_WRITE; }

  virtual track_base_impl* schema_instance() const { return new track_kpf_activity_type(); }

  virtual bool inspect_file( const std::string& fn ) const;

  virtual bool read( const std::string& fn,
                     track_handle_list_type& tracks ) const;

  virtual bool read( std::istream& is,
                     track_handle_list_type& tracks ) const;

  virtual bool write( const std::string& fn,
                      const track_handle_list_type& tracks ) const;

  virtual bool write( std::ostream& is,
                      const track_handle_list_type& tracks ) const;

};

} // ...track_oracle
} // ...kwiver

#endif
