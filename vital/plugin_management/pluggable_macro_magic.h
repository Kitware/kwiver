// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef PLUGGABLE_MACRO_MAGIC_H
#define PLUGGABLE_MACRO_MAGIC_H

#include <vital/cpp_magic.h>
#include <vital/plugin_management/pluggable.h>

#include <utility>

// -----------------
// macro impl config helpers

/*
 * macro for implementations to register a mapping of configurable attributes
 * to config_block properties
 * would need to include:
 *    constructor-param name
 *    description string
 *    default value
 *    constructor param position somehow? maybe?
 *
 * Python equivalents
 * * from_config:
 *     Draw from constructor parameter introspection for
 *
 */

#define TEST_OPT_2( a, b ) a ## b
#define TEST_OPT_3( a, b, c ) a ## b ## c

#define TEST_OPT_ARG( a, b, ... ) \
  IF_ELSE( NOT( HAS_ARGS( __VA_ARGS__ ) ) ) \
  (                               \
    TEST_OPT_2( a, b ),            \
    TEST_OPT_3( a, b, __VA_ARGS__ ) \
  )

int _test_opt_arg{ TEST_OPT_ARG( 1, 2, ) };

// ----------------------------------------------------------------------------
// Helper macros

/**
 * Standard translation of a parameter name to the local member variable
 * the value is stored.
 *
 * This uses the standard prefix "c_" to denote that it is a configured
 * parameter, i.e. one that will be stored in the config_block.
 */
#define CONFIG_VAR_NAME( name ) c_ ## name

/**
 * Conditionally surround the symbol with comments if the second argument is
 * true.
 */
#define MAYBE_COMMENT( symbol, do_comment ) \
  IF_ELSE( BOOL( do_comment ) )             \
  (                                         \
    /* symbol */,                           \
    symbol                                  \
  )

// ----------------------------------------------------------------------------
// Parameter declaration macros
//
// These macros provide options in declaration, translating those variations
// into a standard structure for the rest of this system to utilize.
//
// Common "parameter" tuple structure format:
//   ( name, type, description_str, default_value )
//
//   Required: name, type, description_str
//   Optional: default_value

/**
 * Declare a parameter with no default value.
 */
#define PARAM( name, type, description_str ) \
  ( name, type, description_str, )

/**
 * Declare a parameter *with* a default value.
 */
#define PARAM_DEFAULT( name, type, description_str, default ) \
  ( name, type, description_str, default )

// ----------------------------------------------------------------------------

#define PARAM_VAR_DEF( tuple ) PARAM_VAR_DEF_ tuple
#define PARAM_VAR_DEF_( name, type, description_str, default_value ) \
  type CONFIG_VAR_NAME( name );

#define PARAM_PUBLIC_GETTER( tuple ) PARAM_PUBLIC_GETTER_ tuple
#define PARAM_PUBLIC_GETTER_( name, type, description_str, default_value ) \
  type const& CAT( get_, name )( ) const                                   \
  {                                                                        \
    return this->CONFIG_VAR_NAME( name );                                  \
  }

/**
 * Produce a constructor parameter definition and optional default value
 * assignment.
 */
#define PARAM_CONSTRUCTOR_ARGS( tuple ) PARAM_CONSTRUCTOR_ARGS_ tuple
#define PARAM_CONSTRUCTOR_ARGS_( name, type, description_str, default_value ) \
  IF_ELSE( HAS_ARGS( default_value ) )                                        \
  (                                                                           \
    type name = ( default_value ),                                            \
    type name                                                                 \
  )

/**
 * Produce a constructor default assignment, e.g. that placed after the ":" and
 * before the constructor function body.
 */
#define PARAM_CONSTRUCTOR_ASSN( tuple ) PARAM_CONSTRUCTOR_ASSN_ tuple
#define PARAM_CONSTRUCTOR_ASSN_( name, type, description_str, default_value ) \
  CONFIG_VAR_NAME( name )( name )

/**
 * Produce an access call to the config_block (assumed variable `cb`) to get a
 * value out, cast to the parameters declared type.
 */
#define PARAM_CONFIG_GET( tuple ) PARAM_CONFIG_GET_ tuple
#define PARAM_CONFIG_GET_( name, type, description_str, default ) \
  IF_ELSE( HAS_ARGS( default ) )                                  \
  (                                                               \
    cb.get_value< type >( #name, default ),                       \
    cb.get_value< type >( #name )                                 \
  )

/**
 * Produce a set_value call on the config_block (assumed variable `cb`) to set
 * the current class-variable value.
 */
#define PARAM_CONFIG_DEFAULT_SET( tuple ) PARAM_CONFIG_DEFAULT_SET_ tuple
#define PARAM_CONFIG_DEFAULT_SET_( name, type, description_str, \
                                   default )                    \
  IF_ELSE( HAS_ARGS( default ) )                                \
  (                                                             \
    cb.set_value( #name, default, description_str ); ,          \
    cb.set_value( #name, type(), description_str );             \
  )

// ----------------------------------------------------------------------------

/**
 * The following generation macros hinge on providing an x-macro that expands
 * to enumerate PARAM_* specification tuples as created above.
 */

/**
 * Setup private member variables for the parameter set, as well as public
 * accessor methods that return const& variants of parameter types.
 */
#define PLUGGABLE_VARIABLES( ... ) \
private:                           \
  MAP( PARAM_VAR_DEF, EMPTY, __VA_ARGS__ ) \
public:                            \
  MAP( PARAM_PUBLIC_GETTER, EMPTY, __VA_ARGS__ )

#define PLUGGABLE_CONSTRUCTOR( class_name, ... ) \
public:                                          \
  explicit class_name( MAP( PARAM_CONSTRUCTOR_ARGS, COMMA, __VA_ARGS__ ) ) \
  IF( HAS_ARGS( __VA_ARGS__ ) )(                 \
    : MAP( PARAM_CONSTRUCTOR_ASSN, COMMA, __VA_ARGS__ )                    \
    )                                            \
  {}

#define PLUGGABLE_STATIC_FROM_CONFIG( class_name, ... ) \
public:                                                          \
  static pluggable_sptr from_config( ::kwiver::vital::config_block const& cb ) \
  {                                                              \
    return std::make_shared< class_name >(                       \
      MAP( PARAM_CONFIG_GET, COMMA, __VA_ARGS__ )             \
      );                                                         \
  }

#define PLUGGABLE_STATIC_GET_DEFAULT( ... ) \
public:                                     \
  static void get_default_config( ::kwiver::vital::config_block& cb ) \
  {                                         \
    MAP( PARAM_CONFIG_DEFAULT_SET, EMPTY, __VA_ARGS__ )               \
  }

// ----------------------------------------------------------------------------

/**
 * Define necessary static methods for pluggable interfaces.
 *
 * \param interface_name The name of the interface class, or other like string
 * that will be used as the string name for this interface.
 */
#define PLUGGABLE_INTERFACE( name ) \
public:                             \
  static std::string interface_name() { return #name; }

/**
 * Basic implementation class helper macro for when you want to author your
 * own from_config and get_default_config static methods.
 */
#define PLUGGABLE_IMPL_BASIC( class_name, description ) \
public:                                                 \
  static std::string plugin_name() { return #class_name; } \
  static std::string plugin_description() { return description; }

/**
 * All together now: TODO detail composition
 */
#define PLUGGABLE_IMPL( class_name, description, ... ) \
  PLUGGABLE_VARIABLES( __VA_ARGS__ )                   \
  PLUGGABLE_CONSTRUCTOR( class_name, __VA_ARGS__ )     \
  PLUGGABLE_IMPL_BASIC( class_name, description )      \
  PLUGGABLE_STATIC_FROM_CONFIG( class_name, __VA_ARGS__ ) \
  PLUGGABLE_STATIC_GET_DEFAULT( __VA_ARGS__ )

// ----------------------------------------------------------------------------

namespace kwiver::vital {

class test_interface : public pluggable
{
public:
  PLUGGABLE_INTERFACE( test_interface )

  virtual std::string test() = 0;
};
typedef std::shared_ptr< test_interface > test_interface_sptr;

// ----------------------------------------------------------------------------

/**
 * This impl shows use of more explicit generator macros.
 */
class test_impl_simple : public test_interface
{
public:
  PLUGGABLE_IMPL_BASIC(
    test_impl_simple,
    "This is a simple implementation with no parameters."
    )

  PLUGGABLE_VARIABLES()

  PLUGGABLE_CONSTRUCTOR( test_impl_simple )

  PLUGGABLE_STATIC_FROM_CONFIG(
    test_impl_simple,
    )

  PLUGGABLE_STATIC_GET_DEFAULT()

  std::string
  test() override
  {
    return "simple impl";
  }
};

// ----------------------------------------------------------------------------

class test_impl_parameterized : public test_interface
{
public:
//  // Define an x-macro to enumerate parameter structures.
// #define PARAM_SET() \
//   PARAM( a, int, "some integer" ), \
//   PARAM_DEFAULT( b, std::string, "some string", "foo" )
//
//   // Expand x-macro contents into the thing that wants parameter structures.
//   PLUGGABLE_IMPL(
//     test_impl_parameterized,
//     "This is a test plugin using nesting",
//     PARAM_SET()
//     )

  // or just do it inline if we're only providing it once anyway...
  PLUGGABLE_IMPL(
    test_impl_parameterized,
    "This is a test plugin using nesting",
    // parameters
    PARAM( a, int, "some integer" ),
    PARAM_DEFAULT( b, std::string, "some string", "foo" )
    )

  std::string
  test() override
  {
    std::stringstream ss;
    ss << "class with parameters like " << c_a << " and '" + c_b + "'.";
    return ss.str();
  }
};

// ----------------------------------------------------------------------------

} // end namespace kwiver::vital

#endif //PLUGGABLE_MACRO_MAGIC_H
