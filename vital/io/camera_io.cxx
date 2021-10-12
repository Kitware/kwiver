/*ckwg +29
 * Copyright 2014-2018 by Kitware, Inc.
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
 * \file
 * \brief Implementation of file IO functions for a \ref kwiver::vital::camera
 *
 * File format is the KRTD file.
 */

#include "camera_io.h"

#include <fstream>

#include <vital/exceptions.h>
#include <kwiversys/SystemTools.hxx>

namespace kwiver {
namespace vital {


/// Read in a KRTD file, producing a camera object
camera_perspective_sptr
read_krtd_file( path_t const& file_path )
{
  // Check that file exists
  if ( ! kwiversys::SystemTools::FileExists( file_path ) )
  {
    VITAL_THROW( file_not_found_exception, file_path, "File does not exist." );
  }
  else if (  kwiversys::SystemTools::FileIsDirectory( file_path ) )
  {
    VITAL_THROW( file_not_found_exception, file_path,
                 "Path given doesn't point to a regular file!" );
  }

  // Reading in input file data
  std::ifstream input_stream( file_path.c_str(), std::fstream::in );
  if ( ! input_stream )
  {
    VITAL_THROW( file_not_read_exception, file_path,
                 "Could not open file at given path." );
  }

  // Read the file
  simple_camera_perspective* cam = new simple_camera_perspective();
  input_stream >> *cam;
  return camera_perspective_sptr(cam);
}


/// Read in a KRTD file, producing a camera object
camera_perspective_sptr
read_krtd_file( path_t const& image_file, path_t const& camera_dir )
{
  std::string adj_path =
    camera_dir
    + "/"
    + kwiversys::SystemTools::GetFilenameWithoutLastExtension( image_file );

  return read_krtd_file( path_t( adj_path.append( ".krtd" ) ) );
}


/// Output the given \c camera object to the specified file path
void
write_krtd_file( camera_perspective const&  cam,
                 path_t const&  file_path )
{

  // If the given path is a directory, we obviously can't write to it.
  if ( kwiversys::SystemTools::FileIsDirectory( file_path ) )
  {
    VITAL_THROW( file_write_exception, file_path,
          "Path given is a directory, can not write file." );
  }

  // Check that the directory of the given filepath exists, creating necessary
  // directories where needed.
  std::string parent_dir = kwiversys::SystemTools::GetFilenamePath(
    kwiversys::SystemTools::CollapseFullPath( file_path ));
  if ( ! kwiversys::SystemTools::FileIsDirectory( parent_dir ) )
  {
    if ( ! kwiversys::SystemTools::MakeDirectory( parent_dir ) )
    {
      VITAL_THROW( file_write_exception, parent_dir,
           "Attempted directory creation, but no directory created! No idea what happened here..." );
    }
  }

  // open output file and write the ins_data
  std::ofstream ofile( file_path.c_str() );
  ofile << cam;
  ofile.close();
}


} } // end of namespace
