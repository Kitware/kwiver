Uncrustify
==========
Uncrustify is a tool that is used to check and apply the configured C++ code
style.

Getting Uncrustify
------------------
For local use, it is recommended to build the tool from source.
This is usually because your system package manager has registered an outdated
version as the package is actively maintained and received frequent feature
and bug-fix updates.

The repository is located `here <https://github.com/uncrustify/uncrustify>`_.
The build instructions may be found in it's top-level ``README.md`` file.

Below are example commands to clone, build and locally install uncrustify for
Unix machines with Makefiles.
The installation target below is the `~/.local/` installation root, which for
many systems is the user-local, non-root-user installation root.[

    $ git clone git@github.com:uncrustify/uncrustify.git .
    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/.local ..
    $ make install -j4

Running On Sources
------------------
Uncrustify (on purpose) does not provide any logic regarding finding what files
to run on, so it is left to an external agent to provide it files.

For unix systems, we provide a helper script ``scripts/unix_uncrustify.sh``
that will find C++ files under applicable directories and run ``uncrustify`` on
them.
See the script for more details.

Intentionally Un-Checked Code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A select few areas of this codebase are not expected to follow our style
requirements:

* KWSys nested library -- ``vital/kwiversys``

  * This is effectively a bundled third-party library that we do not intend to
    modify.

CLion Plugin
^^^^^^^^^^^^
If CLion happens to be your IDE of choice, there happens to be an available
free plugin on the market place that integrates uncrustify with the "Reformat
File" functionality.
The plugin is simply called "Uncrustify" and can be found by opening Settings
--> Plugins --> Marketplace, searching for the name and clicking the "Install"
button.
The JetBrains Marketplace webpage for this plugin is `here
<https://plugins.jetbrains.com/plugin/17528-uncrustify>`_.

The settings for this plugin can now be found under Settings --> Tools -->
Uncrustify.
Here you should input the absolute path to the ``uncrustify`` binary as well as
the path to the ``.uncrustify.cfg`` configuration file located in the root of
this repository.

To apply the uncrustify configured settings when reformatting a file, make sure
the "Whole file" mode is selected ("Reformat File...", or Ctrl+Alt+Shift+L),
and apply a reformat to the currently focused file with "Reformat Code", or
Ctrl+Alt+L.

Selectively Disabling Checking
------------------------------
Uncrustify has a means of blocking out lines or regions of code that
**should not** be checked or modified by the tool.
The current configuration is set to use the ``UNCRUST-OFF`` and UNCRUST-ON``
strings.
