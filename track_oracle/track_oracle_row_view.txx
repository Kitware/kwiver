/*ckwg +5
 * Copyright 2010-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_oracle_row_view.h"

#include <stdexcept>
#include <sstream>
#include <track_oracle/track_field.h>


using std::ostringstream;
using std::runtime_error;
using std::string;


namespace kwiver {
namespace track_oracle {

template< typename T>
track_field<T>&
track_oracle_row_view
::add_field( const string& field_name )
{
  track_field<T>* new_field = new track_field<T>( field_name, this );
  // ensure we're not duplicating fields
  for (size_t i=0; i<this->field_list.size(); ++i)
  {
    if (this->field_list[i]->get_field_handle() == new_field->get_field_handle())
    {
      ostringstream oss;
      oss << "track_field::add_field called with '" << field_name << "' duplicates existing field handle";
      throw runtime_error( oss.str() );
    }
  }
  this->field_list.push_back( new_field );
  this->this_owns_ptr.push_back( true );
  return *new_field;
}

} // ...track_oracle
} // ...kwiver
