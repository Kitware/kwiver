Plugin Management
=================

KWIVER uses runtime plugins to keep Vital independent from the libraries that
implement concrete algorithms, processes, applets, and other extension points.
Application code normally talks to abstract Vital interfaces, then asks the
plugin manager to discover implementations and instantiate the one selected by
name or configuration.

The primary entry point is ``kwiver::vital::plugin_manager`` in
``vital/plugin_management/plugin_manager.h``.
It is a singleton front-end around the lower-level ``plugin_loader``.
Most application code should use the singleton and avoid constructing loaders
directly.

Basic Loading Flow
------------------

A typical C++ application loads plugins once during startup, then creates
algorithm implementations:

.. code-block:: c++

   #include <vital/algo/image_io.h>
   #include <vital/plugin_management/plugin_manager.h>

   namespace kv = kwiver::vital;

   int main()
   {
     auto& plugins = kv::plugin_manager::instance();

     // Optionally add additional plugin module paths to search in.
     plugins.add_search_path( "/path/to/kwiver/plugins" );

     plugins.load_all_plugins();

     // Convenience function for creating instances of algorithm-type plugins.
     // Includable from "vital/algo/algorithm.txx".
     auto reader = kv::create_algorithm< kv::algo::image_io >( "vxl" );
   }

``plugin_manager`` initializes its search path from the configured KWIVER module
locations and from the ``KWIVER_PLUGIN_PATH`` environment variable.
Calls to ``add_search_path()`` append additional roots before loading.

By default, ``load_all_plugins()`` scans the standard subdirectories under each
search root:

* ``processes``
* ``algorithms``
* ``applets``
* ``plugin_explorer``
* ``modules``

The loaded shared libraries are expected to export the registration function
``register_factories``.
When a library is opened, that function registers one or more
``plugin_factory`` instances with the active loader.

Selecting an Implementation
---------------------------

Loading makes factories available; it does not choose which implementation your
code will use.
Selection happens later by asking for an implementation name.
For algorithms, this is usually done through the helper API in the algorithm
interface:

.. code-block:: c++

   plugins.load_all_plugins( kv::plugin_manager::plugin_type::ALGORITHMS );

   auto detector =
     kv::create_algorithm< kv::algo::image_object_detector >(
       "example_detector" );

For lower-level pluggable interfaces, ``plugin_manager.h`` provides
``implementation_factory_by_name``:

.. code-block:: c++

   using factory_t =
     kv::implementation_factory_by_name< my_interface >;

   factory_t factory;
   auto object = factory.create( "my_implementation", config );

The implementation name is stored on the factory as
``plugin_factory::PLUGIN_NAME``.
Names must be unique for a given interface.
If a name is not available, creation fails with a ``plugin_factory_not_found``
exception.

To inspect what names are currently registered for an interface, use
``impl_names<T>()``:

.. code-block:: c++

   for( auto const& name :
        plugins.impl_names< kv::algo::image_object_detector >() )
   {
     std::cout << name << '\n';
   }

Loading Less Than Everything
----------------------------

``plugin_manager`` has coarse loading controls, but it does not currently expose
a supported "load exactly this one algorithm implementation" workflow.

The available controls are:

* Load only broad plugin classes with ``load_all_plugins(types)``.
* Load plugins from explicit directories with ``load_plugins(path_list_t)``.
* Restrict discovery by controlling the search path and directory contents.

For example, code that only needs algorithms can avoid scanning process and
applet directories:

.. code-block:: c++

   auto& plugins = kv::plugin_manager::instance();
   plugins.load_all_plugins(
     kv::plugin_manager::plugin_type::ALGORITHMS );

The ``plugin_type`` values can be combined as bitflags:

.. code-block:: c++

   using plugin_type = kv::plugin_manager::plugin_type;

   plugins.load_all_plugins(
     plugin_type::ALGORITHMS | plugin_type::APPLETS );

If you need tighter control, point the manager at a directory that contains
only the plugin libraries you are willing to load:

.. code-block:: c++

   kv::path_list_t plugin_dirs;
   plugin_dirs.push_back( "/path/to/minimal/plugin-set" );

   kv::plugin_manager::instance().load_plugins( plugin_dirs );

This last example scans every shared library in provided directories.
It is selective by directory, not by individual factory.

When to Reload
--------------

The first call to ``load_all_plugins()`` records which broad plugin categories
have been loaded.
Repeated calls with the same category do not rescan those directories.
Use ``reload_all_plugins()`` only when you intentionally want to clear the
registered factories, rebuild the underlying loader, and scan again.

Reloading is a process-wide operation because the manager is a singleton.
Avoid using it as routine per-operation control flow.

About ``plugin_loader_filter``
-------------------------------

There are filter types under ``vital/plugin_management``:

* ``plugin_loader_filter``
* ``plugin_filter_category``
* ``plugin_filter_default``

Their names suggest library-level or factory-level filtering, and
``plugin_loader`` still has ``add_filter()`` and ``clear_filters()`` methods.
However, the current loader implementation does not apply those filters during
library loading or factory registration.
The relevant hook points in ``plugin_loader.cxx`` are commented as previously
existing filter steps that were never effectively used.

Practically, this means ``plugin_loader_filter`` is not a usable way to request
"load only this algorithm" through ``plugin_manager`` today.
Document and use the coarse controls above unless filter support is restored in
the loader and exposed through a supported manager API.
