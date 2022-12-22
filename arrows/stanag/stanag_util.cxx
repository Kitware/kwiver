// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_util.h"
#include <iostream>

namespace klv = kwiver::arrows::klv;

namespace kwiver {

namespace arrows {

namespace stanag {

// ----------------------------------------------------------------------------
std::string
trim_whitespace( std::string input )
{
  if( input == "" ) { return ""; }

  const auto str_begin = input.find_first_not_of( " \t" );
  const auto str_end = input.find_last_not_of( " \t" );

  return input.substr( str_begin, ( str_end - str_begin ) + 1 );
}


} // namespace stanag

} // namespace arrows

} // namespace kwiver
