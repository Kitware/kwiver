
# Building
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

# What Scikit-Build Does
Scikit-build will perform a "normal build," however "installed" items will be
placed such that they are included under the package's root module.
For example, if we install the file `<prefix>/lib/foo.so`, then the package
built by Scikit-build will include `<site-packages>/kwiver/lib/foo.so`.

Note that certain build and installation locations are different for
scikit-build runs vs. "normal" build runs.
This is done in order to facilitate proper library and module library build and
installation locations for python packages.
This is managed by the KWIVER CMake functions and should not need additional
management by binding developers.

Example: `kwiver_add_python_library` may result in, for a "normal" build, a
library placed into
`<build_dir>/lib/python3.6/site-packages/kwiver/vital/config/_config.so`
however for a scikit-build run, the same library would be placed in
`<source_dir>/_skbuild/.../cmake-build/vital/config/_config.so/`.
