// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * @file
 * @brief Factory registration function for exposing python-defined
 * implementations.
 */

#include <pybind11/pybind11.h>

#include <vital/logger/logger.h>
#include <vital/plugin_management/plugin_loader.h>

#include <python/kwiver/internal/python.h>
#include <python/kwiver/internal/python_plugin_factory.h>

#include <python/kwiver/vital/plugins/plugins_from_python_export.h>

namespace kv = ::kwiver::vital;
namespace py = pybind11;

// ----------------------------------------------------------------------------
// Helper Functions

/**
 * @brief Initialize a python interpreter if one is not already.
 *
 * This function is idempotent.
 */
static void check_and_initialize_python_interpreter();

// ----------------------------------------------------------------------------
// Registration Function
extern "C"
[[maybe_unused]] PLUGINS_FROM_PYTHON_EXPORT

void
register_factories( ::kv::plugin_loader& vpl )
{
  ::kv::logger_handle_t log = ::kv::get_logger(
    "python.kwiver.vital.plugins.register_factories"
  );

  // TODO: Hook to skip python plugin registration.
  //   !!! This would explicitly be before potentially starting the
  //       interpreter. Base on the presence of an environment variable.
  //       Was previously "SPROKIT_NO_PYTHON_MODULES", should be something more
  //       applicable (this isn't sprokit specific).

  // Make sure there is an interpreter running.
  check_and_initialize_python_interpreter();

  // In upstream, in this slot there was logic to dynamically load the
  // `libpython*.so` library here.
  // Ameya explained anecdotally that without this, the following plugin
  // loading block will raise segfaults due to symbol not found errors.
  // If we find this to still be true, then reinstate this logic.
  // * First, just reinstate the portion that loaded the library as
  //   introspected from the interpreter, NOT from the environment
  //   PYTHON_LIBRARY variable.
  // * Upstream code specifically used unix `dlopen` instead of KWSYS tool in
  //   order to pass the `RTLD_GLOBAL` flag that KWSYS does not. Ameya reported
  //   getting faults otherwise.
  //   * Modify local version of KWSYS to also pass the `RTLD_GLOBAL` flag?

  // Generate factories to add to `vpl`.
  py::object const mod_discovery =
    py::module::import( "kwiver.vital.plugins.discovery" );

  // DEBUG IMPORT FOR CONCRETE
  py::object const mod_debug_impls =
    py::module::import( "kwiver.vital.test_interface.python_say" );
  // DEBUG IMPORT FOR CONCRETE

  py::list python_concrete_vec =
    mod_discovery.attr( "_get_concrete_pluggable_types" )();
  for( auto const& o : python_concrete_vec )
  {
    LOG_DEBUG( log, o.attr("__name__").cast<std::string>() );
//    kv::python::python_plugin_factory
  }

}

// ----------------------------------------------------------------------------
// Helper function implementations
void
check_and_initialize_python_interpreter()
{
  ::kv::logger_handle_t log = ::kv::get_logger(
    "python.kwiver.vital.plugins.check_and_initialize_python_interpreter"
    );

  // Check if a python interpreter already exists, so we don't clobber sys.argv
  // (e.g. if sprokit is initialized from python)
  if( !Py_IsInitialized() )
  {
    LOG_DEBUG( log, "Initializing python interpreter" );
    // Embed a python interpreter if one does not exist
    Py_Initialize();

    // Set Python interpreter attribute: sys.argv = []
    // parameters are: (argc, argv, update-path)
    PySys_SetArgvEx( 0, nullptr, 0 );
  }

  // Let pybind11 initialize threads and set up its internal data structures if
  // not already done so.
  if( !PyEval_ThreadsInitialized() )
  {
    LOG_DEBUG( log, "Python threads not initialized yet, letting pybind11 do "
                    "it's thing." );
    {
      pybind11::detail::get_internals();
    }
    // Release the GIL
    PyEval_SaveThread();
  }
}
