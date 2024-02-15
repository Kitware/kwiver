# Untouched parts brought in
A number of parts from mainline kwiver have been of course copied into here into
equivalent locations due to the size and complexity of the library basis.
Here is a list of the parts that were more or less taken "as is" from upstream
KWIVER, modulo "fixes" and minor updates for clarity and documentation.

* CMake/
* tests/
* vital/config/
* vital/exceptions/
* vital/kwiversys/
* vital/logger/
  * Added separation of pluigins into `vital/logger_plugins/`
* vital/util/

# Things specifically changed/removed from KWIVER 1.x
* Updated base CXX standard --> **17**.

# Dangling TODOs
* Change `vital_vpm` naming to something more redundant, e.g. `vital_pm`, or
  `vital_plugins`
* Use python poetry.
  * Use with scikit-build potential: https://github.com/tueda/test-poetry-scikit-build


# NEXT STEPS

Figure out plugin stuff for python
* probably want to expose "pluggable" type.
* expose plugin manager singleton? just methods?

## notes on upstream python binding contents
`modules.cxx` is pybind11 exposing singleton instance methods.

`registration.cxx` is for creating a plugin library (à la `register_factories`)
that ... FIGURE THIS OUT.
* There is an aspect if calling into python to extend filesystem search paths
  for addition to the plugin_manager for .so library loading: modules contained
  within the package and maybe provided by other packages?
    * Could this be simpler? There's a lot of back and forth in trying to figure
      out what's happening here.

`module_loader.py` gets used only by `registration.cxx` to call into the
`__vital_algorithm_register__` method shenanigans
* THIS IS LIKELY THE MAIN REPLACEMENT AREA FOR SMQTK METHOD
* this currently *does* use entrypoint stuff to some extent

## sketch of python plugin loading
0. [load-all-plugins (python)]
1. load-all-plugins (c++)
2. load .so modules
    1. ...
    2. load python-hook .so register_factories (given plugin_loader inst)
        1. setup python, etc. ...
        2. call into python to discover ... impls ... so I also need to know the
           interface "name" with the current registration schema
           1. get modules (entry-points) under set namespace
           2. get modules from env var
           3. retain those that are "not abstract"
           4. for each...
              1. wrap in PythonFactory subclass
                 * sets INTERFACE_TYPE to attr("interface_name")()
                 * sets CONCRETE_TYPE to `__module__ + __name__`
                 * sets PLUGIN_NAME to `__name__`
                 * sets PLUGIN_DESCRIPTION to `__doc__` maybe?
              2. register with vpl via python factory subclass.

Could really use some modification of Pluggable base-class binding to
expose/require "abstract" instance/class-methods.
- trouble/cannot inject abc.ABCMeta metaclass in bindings layer
