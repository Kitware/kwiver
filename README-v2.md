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


# Things specifically changed/removes
* Removed `vital/any.h` with the updated base CXX standard --> **17**.


# Dangling TODOs
* Change `vital_vpm` naming to something more redundant, e.g. `vital_pm`, or
  `vital_plugins`


# Plugin management modifications
## Plugin Factory
Added a `pluggable` type to act as a base class handle for pluggalbe things so
that the factory could define a pure-virtual for instance creation.

Changed the instance creation method in to take in a config block instead of
being empty.
The concrete, templated implementation class of the base factory interface,
concrete_plugin_factory, is header only and requires:
  * the concrete class inherit from the given interface class
  * that the concrete have the static methods `from_config` and
    `get_default_config`. These are documented in the `plugin_factory`
    -equivalent methods.

Standardizing on plugin interface and concrete names being that of `typeid().name()`
result.

## Plugin Loader
Removed plugin filters. Linus described these as deprecated in lieu of the use
of plugin types at the manager level.

"Deprecated" a number of methods on the loader that don't seem to be used in
ernest.
I may be wrong about these and just have only commented them out for now.

## Plugin Manager
Similarly, deprecated a number of functions as in the loader.
Same comment about my potential wrongness.

Did not carry over yet the "internal" sub-class just yet, also pending exposure
of a reason for existence.

# Plugin Use Docs
## Using Tools
Register plugins by running
```c++
kwiver::vital::plugin_manager::instance()->load_all_plugins();
```
Additional search paths may be added for discovery.
This may be done programatically via the `plugin_manager` instance or via
the environment variable `KWIVER_PLUGIN_PATH`.
The environment viariable should be filesystem paths to directories, separated
by your system's usual PATH-separator.

## Registering Plugins
In your CMake, use `kwiver_add_plugin` to create shared plugin libraries.
This library should be given at least a source file that defines an exter-C
function `void register_factories( kwiver::vital::plugin_loader& vpl )` that
adds factories to the given loader instance.
<< EXPAND >>

When registering plugins, the most common method of doing so is by
```c++
vpl->add_factory<INTERFACE, CONCRETE>( "plugin-name" );
```

# Python
## Building
Python Requirements
`pip install -r requirements/dev.txt`

We assume CMake is available on the system.
Otherwise, CMake is also installable via pip.
Will be installed via above requirements file.

```bash
python3 setup.py bdist_wheel \
  -- -Dfletch_DIR=<PATH_TO_FLETCH_BUILD_OR_INSTALL> \
  -- -j$(nproc)
```

This should create a `dist/` directory here after successful completion.
In that directory will be the wheel file for the python version built against.
