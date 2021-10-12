Configuration File Format
=========================

Configuration files are used to establish a key, value store that is
available within a program. The entries can be grouped in a hierarchy
or blocks to aide in constructing complex configurations. This
document describes the format and features of config file.

Syntax
------
Configuration Entries
'''''''''''''''''''''
Configuration entires are in a < key > = < value > format. The key
specifies a name for the entry that is assigned the value. All values
are treated as strings. No interpretation is done when reading
configuration entries. All leading and trailing spaces are removed
from the *value* string. Spaces embedded in the *value* portion are
retained.

If the *value* string is enclosed in quotes, the quotes will become
part of the value and passed to the program.

The simplest form of a config entry is:::

  simple = value


Configuration entries can be grouped so that entries for a specific
can be specified as a subblock. For example configuration items for
the *foo* algorithm can be specified as::

  foo:mode = red
  foo:sync = false
  foo:debug = false

by prepending the block/subblock name before the name with a ":"
separator. All conrig entries for *foo* can be extracted from the
larger config into a subblock that is expected by the
algorithm. Blocks can be nested to an arbitrary depth, as shown below.::

  foo:bar:baz:arf:mode = blue

A configuration entry can be made read-only bp appending *[RO]* to the
key string. Once an entry has been declared a read only, it cannot be
assigned another value or deleted from the config.::

  simple[RO] = value

Comments
''''''''
Comments start wth the '#' character and continue to the end of line.
When a comment appears after a configuration value,

Block Specification
'''''''''''''''''''
In some cases the fully qualified configuration key can become long and unwieldy.
The block directive can be used to establish a configuration context to be applied
to the enclosed configuration entries.::

  block alg

Starts a block with the *alg* block name and all entries within the block will
have ``alg:`` prepended to the entry name.::

  block alg
     mode = red      # becomes alg:mode = red
  endblock

Blocks can be nested to an arbitrary depth with each providing context
for the enclosed entries.::

  block foo
    block bar:fizzle
      mode = yellow     # becomes foo:bar:fizzle:mode = yellow
    endblock
  endblock


Including Files
'''''''''''''''
The include directive logically inserts the contents of the specified
file into the current file at the point of the include
directive. Include files provide an easy way to break up large
configurations into smaller reusable pieces. ::

  include    a_file

If the file name is not an absolute path, it is located by scanning
the current config search path.  The manner in which the config
include path is created is described in a following section.  If the
file is still not found, the stack of include directories is scanned
from the current include file back to the initial config file. Macro
substitution, as described below, is performed on the file name string
before the searching is done.

Block specifications and include directives can be used together to
build reusable and shareable configuration snippets.::

  block main
    block alg_one
      include alg_foo.config
    endblock

    block alg_two
      include alg_foo.config
    endblock
  endblock

In this case the same configuration structure can be used in two
places in the overall configuration.

Include files can be nested to an arbitrary depth.

Relativepath Modifier
'''''''''''''''''''''
There are cases where an algorithm needs an external file containing
binary data that is tied to a specific configuration.  These data
files are usually stored with the main configuration files.
Specifying a full hard coded file path is not portable between
different users and systems.

The solution is to specify the location of these external files
relative to the configuration file and use the *relativepath* modifier
construct a full, absolute path at run time by prepending the
configuration file directory path to the value.::

  relativepath data_file = ../data/online_dat.dat

If the current configuration file is
``/home/vital/project/config/blue/foo.config``, the resulting config
entry for **data_file** will be
``/home/vital/project/config/blue/../data/online.dat``

The *relativepath* modifier can be applied to any configuration entry,
but it only makes sense to use it with relative file specifications.

Config File Include Path
------------------------
Config file search paths are constructed differently depending on the target platform.
The directories are searched in the order specified in the following sections.

Windows Platform
''''''''''''''''
- .  (the current working directory
- ${KWIVER_CONFIG_PATH}          (if set)
- $<CSIDL_LOCAL_APPDATA>/<app-name>[/<app-version>]/config
- $<CSIDL_APPDATA>/<app-name>[/<app-version>]/config
- $<CSIDL_COMMON_APPDATA>/<app-name>[/<app-version>]/config
- <install-dir>/share/<app-name>[/<app-version>]/config
- <install-dir>/share/config
- <install-dir>/config

OS/X Apple Platform
'''''''''''''''''''
- .  (the current working directory
- ${KWIVER_CONFIG_PATH}                                    (if set)
- ${XDG_CONFIG_HOME}/<app-name>[/<app-version>]/config     (if $XDG_CONFIG_HOME set)
- ${HOME}/.config/<app-name>[/<app-version>]/config        (if $HOME set)
- /etc/xdg/<app-name>[/<app-version>]/config
- /etc/<app-name>[/<app-version>]/config
- ${HOME}/Library/Application Support/<app-name>[/<app-version>]/config (if $HOME set)
- /Library/Application Support/<app-name>[/<app-version>]/config
- /usr/local/share/<app-name>[/<app-version>]/config
- /usr/share/<app-name>[/<app-version>]/config

If <install-dir> is not ``/usr`` or ``/usr/local``:

- <install-dir>/share/<app-name>[/<app-version>]/config
- <install-dir>/share/config
- <install-dir>/config
- <install-dir>/Resources/config

Other Posix Platforms (e.g. Linux)
''''''''''''''''''''''''''''''''''
- .  (the current working directory
- ${KWIVER_CONFIG_PATH}                                    (if set)
- ${XDG_CONFIG_HOME}/<app-name>[/<app-version>]/config     (if $XDG_CONFIG_HOME set)
- ${HOME}/.config/<app-name>[/<app-version>]/config        (if $HOME set)
- /etc/xdg/<app-name>[/<app-version>]/config
- /etc/<app-name>[/<app-version>]/config
- /usr/local/share/<app-name>[/<app-version>]/config
- /usr/share/<app-name>[/<app-version>]/config

If <install-dir> is not ``/usr`` or ``/usr/local``:

- <install-dir>/share/<app-name>[/<app-version>]/config
- <install-dir>/share/config
- <install-dir>/config

The environment variable \c KWIVER_CONFIG_PATH can be set with a list
of one or more directories, in the same manner as the native execution
``PATH`` variable, to be searched for config files.

Macro Substitution
------------------
The values for configuration elements can be composed from static text
in the config file and dynamic text supplied by macro providers. The
format of a macro specification is ``$TYPE{name}`` where **TYPE** is the
name of macro provider and **name** requests a particular value to be
supplied. The **name** entry is specific to each provider.

The text of the macro specification is only replaced. Any leading or
trailing blanks will remain.  If the value of a macro is not defined,
the macro specification will be replaced with the null string.

Macro Providers
'''''''''''''''
The macro providers are listed below and discussed in the following sections.

- LOCAL - locally defined values
- ENV - program environment
- CONFIG - values from current config block
- SYSENV - system environment


LOCAL Macro Provider
''''''''''''''''''''
This macro provider supplies values that have been stored previously
in the config file.  Local values are specified in the config file
using the ":=" operator. For example the config entry ``mode := online``
makes ``$LOCAL{mode}`` available in subsequent configuration
entries.::

  mode := online
  ...
 config_file = data/$LOCAL{mode}/model.dat

This type of macro definition can appear anywhere in a config file and
becomes available for use on the next line.  The current block context
has no effect on the name of the macro.

ENV Macro Provider
''''''''''''''''''
This macro provides gives access to the current program
environment. The values of environment variables such as "HOME" can be
used by specifying ``$ENV{HOME}`` in the config file.

CONFIG Macro Provider
'''''''''''''''''''''
This macro provider gives access to previously defined configuration
entries. For example::

  foo:bar = baz

makes the value available by specifying ``$CONFIG{foo:bar}`` to following lines in the config file
as shown below.::

  value = mode-$CONFIG{foo:bar}ify

SYSENV Macro Provider
'''''''''''''''''''''
This macro provider supports the following symbols derived from the
current host operating system environment.

- cwd - current working directory
- numproc - number of processors in the current system
- totalvirtualmemory - number of KB of total virtual memory
- availablevirtualmemory - number of KB of available virtual memory
- totalphysicalmemory - number of KB of total physical memory
- availablephysicalmemory - number of KB of physical virtual memory
- hostname - name of the host computer
- domainname - name of the computer in the domain
- osname - name of the host operating system
- osdescription - description of the host operating system
- osplatform - platorm name (e.g. x86-64)
- osversion - version number for the host operating system
- iswindows - TRUE if running on Windows system
- islinux - TRUE if running on Linux system
- isapple - TRUE if running on Apple system
- is64bits - TRUE if running on a 64 bit machine
