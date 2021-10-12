// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/types/metadata.h>
#include <vital/types/metadata_traits.h>
#include <vital/types/metadata_tags.h>
#include <vital/types/geo_point.h>
#include <vital/types/geo_polygon.h>
#include <vital/util/demangle.h>

#include <pybind11/pybind11.h>

#include <memory>
#include <string>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace kwiver::vital;

kwiver::vital::any
py_object_to_any( vital_metadata_tag tag, py::object data )
{
  auto const& type = tag_traits_by_tag( tag ).type();
  if( type == typeid( bool ) )
  {
    return data.cast< bool >();
  }
  else if ( type == typeid( uint64_t ) )
  {
    return data.cast< uint64_t >();
  }
  else if ( type == typeid( int ) )
  {
    return data.cast< int >();
  }
  else if ( type == typeid( double ) )
  {
    return data.cast< double >();
  }
  else if ( type == typeid( string_t ) )
  {
    return data.cast< string_t >();
  }
  else if ( type == typeid( geo_point ) )
  {
    return data.cast< geo_point >();
  }
  else if ( type == typeid( geo_polygon ) )
  {
    return data.cast< geo_polygon >();
  }
  else
  {
    throw std::logic_error( "unsupported type" );
  }
}

py::object
any_to_py_object( kwiver::vital::any data )
{
  auto const& type = data.type();
  if( type == typeid( bool ) )
  {
    return py::cast( kwiver::vital::any_cast< bool >( data ) );
  }
  else if ( type == typeid( uint64_t ) )
  {
    return py::cast( kwiver::vital::any_cast< uint64_t >( data ) );
  }
  else if ( type == typeid( int ) )
  {
    return py::cast( kwiver::vital::any_cast< int >( data ) );
  }
  else if ( type == typeid( double ) )
  {
    return py::cast( kwiver::vital::any_cast< double >( data ) );
  }
  else if ( type == typeid( string_t ) )
  {
    return py::cast( kwiver::vital::any_cast< string_t >( data ) );
  }
  else if ( type == typeid( geo_point ) )
  {
    return py::cast( kwiver::vital::any_cast< geo_point >( data ) );
  }
  else if ( type == typeid( geo_polygon ) )
  {
    return py::cast( kwiver::vital::any_cast< geo_polygon >( data ) );
  }
  else
  {
    throw std::logic_error( "unsupported type" );
  }
}

void adder(metadata &self, py::object data, vital_metadata_tag tag )
{
  self.add_any( tag, py_object_to_any( tag, data ) );
}

PYBIND11_MODULE( metadata, m )
{
  py::class_< metadata_item,
              std::shared_ptr< metadata_item > >( m, "MetadataItem" )
  .def( py::init([]( vital_metadata_tag tag, py::object data ){
    return new metadata_item{ tag, py_object_to_any( tag, data ) };
  }) )
  .def( "is_valid",    &metadata_item::is_valid )
  .def( "__nonzero__", &metadata_item::is_valid )
  .def( "__bool__",    &metadata_item::is_valid )
  .def_property_readonly( "name", &metadata_item::name )
  .def_property_readonly( "tag",  &metadata_item::tag )
  .def_property_readonly( "type", [] ( metadata_item const& self )
  {
    // The demangled name for strings is long and complicated
    // So we'll check that case here.
    if ( self.has_string() )
    {
      return std::string( "string" );
    }
    return demangle( self.type().name() );
  })
  .def_property_readonly( "data", []( metadata_item const& self ){
    return any_to_py_object( self.data() );
  } )
  .def( "as_double",  &metadata_item::as_double )
  .def( "has_double", &metadata_item::has_double )
  .def( "as_uint64",  &metadata_item::as_uint64 )
  .def( "has_uint64", &metadata_item::has_uint64 )
  .def( "as_string",  []( metadata_item const& self ) -> string_t {
    if( self.type() == typeid( bool ) )
    {
      return kwiver::vital::any_cast< bool >( self.data() ) ? "True" : "False";
    }
    return self.as_string();
   } )
  .def( "has_string", &metadata_item::has_string )

  // No print_value() since it is almost the same as as_string,
  // except it accepts a stream as argument, which can be pre-configured
  // with a certain precision. Python users obviously won't be able to use this,
  // so we'll just bind as_string.
  ;

  // Now bind the actual metadata class
  py::class_< metadata, std::shared_ptr< metadata > >( m, "Metadata" )
  .def( py::init<>() )
  // TODO: resolve rvalue references in members https://github.com/pybind/pybind11/issues/1694
  .def( "add_copy", (void (metadata::*)(std::shared_ptr<metadata_item const>const &)) &metadata::add_copy)
  // usage: .add(identifier, tag)
  .def( "add",           &adder )
  .def( "erase",         &metadata::erase )
  .def( "has",           &metadata::has )
  .def( "find",          &metadata::find, py::return_value_policy::reference_internal )
  .def( "size",          &metadata::size )
  .def( "empty",         &metadata::empty )
  .def_static( "format_string", &metadata::format_string )
  .def_property( "timestamp",   &metadata::timestamp, &metadata::set_timestamp )
  ;

  m.def( "test_equal_content", &test_equal_content )
  ;
}
#undef REGISTER_TYPED_METADATA
