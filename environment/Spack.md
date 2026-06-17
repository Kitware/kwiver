# Getting Started with Spack and Kwiver

## Spack: A quick lesson


Spack docs: <https://spack.readthedocs.io/en/latest>

Spack is a python based, combinatorial package manager that sits somewhere between system package manager and language package manager. It is multi-lingual, and support very granular control over all of its package's build options.

Spack is able to install any number of the same package with different variations simultaneously without issue, and all variants can peacefully coexist on the same system/environment as needed.

While spack is a package manager and packages are a first class component, most of what the user works with are called "specs" (package specifications), which are abstract representations of a build graph. Note build graph, not package, a single spec can provide specifications and requirements for the entire build graph of a given installation, not just for a single package. The spec syntax is fairly involved and more information can be found [at the spack docs](https://spack.readthedocs.io/en/latest/spec_syntax.html).

Most of what you need to know however is

@ = version
+/~/- disable or enable a variant (+ is enable)

Spack takes those abstract specifictions and runs a solver. This usually takes a while (SAT is hard), but eventually produces an installable concrete build graph.

From there, Spack runs whatever underlying build tools are required to build each package from source, or pulls from a configured binary mirror.

Each installed package is then placed in an install prefix with a hash sufficiently long and unique that each variation of a given package has a prefix guarunteed to be unique.

Spack packages are used by one of two mechanisms:

`spack load` - sets up your environment to be able to discover a given package

Spack environments - this is a bit more powerful than `spack install && spack load`, it allows the specification, configuration, and loading of multiple packages as a unified installation group, or environment, and when used with views, makes the entire corpus of software in an environment available to the user. If you're familiar with conda environments this is similar.

Kwiver will be using Spack's environments as a dependency management strategy, but users can and should feel free to `spack install && spack load` any packages they need ad-hoc whether its to test a new dependency or just use a one off tool. If the need for the package is to update a Kwiver dependency (add a new dep or bump a version), a load for testing is fine, but before upstreaming it needs to first be concretized with the Spack environment for Kwiver to ensure environment consistency and update the lockfile for all other Kwiver devlopers and users.

Spack has a primary package repository at <https://github.com/spack/spack-packages> but also allows for user derived package repositories to exist alongside its own "builtin" repo (builtin is the name we give the core repo, which used to be "builtin" to Spack's source).
Kwiver has such a repo at <https://kitware.gitlab.com/kwiver/fletch-package-repo>.

Spack also has build caches/binary mirrors that allow for the downloading of prebuilt binaries vs always building from source.

Spack hosts a number of these, and we also host one for Kwiver specifically.
This is hosted via the Gitlab Container Registry at gitlab.kitware.com at  gitlab.kitware.com:4567/kwiver/kwiver/kwiver-spack-buildcache

### Setting up Spack

#### Requirements

- Python 3.6+
- Git (any relatively modern version)

#### Installing Spack

```bash
$ git clone https://github.com/spack <installation prefix of choice>
```

You now have Spack installed

#### Setup

Whenever you wish to use Spack, to make Spack available in your shell, invoke

```bash
$ source <path to spack root>/share/spack/setup-env.sh
```

or on Windows you can run

```cmd
> <path to spack root>/bin/spack_cmd.bat
```

Which will launch a new cmd shell with Spack available
you can also just click on that bat file from the file explorer or run

```cmd
> <path to spack root>/share/spack/setup-env.bat|.ps1
```

to setup your current shell (powershell or cmd) for Spack

There are also support for the fish and zsh shells, just invoke the same setup-env script with the appropriate extension for the shell.

You should now be able to run

```bash
spack commands
```

to see a full list of all the CLI commands Spack can run.

### Running Spack

To start using Spack to install and use packages you just need to run two commands:

```bash
$ spack env activate <path to kwiver source tree root>
$ spack install
```

If you wish to use the exact environment used in the CI, you can initalize it like

```bash
$ spack env create fletch-env <kwiver-root>/environments/<platform>/spack.lock
$ spack env activate fletch-env
```

This will generate an environment from the already concretized lockfile.

Kwiver has a Spack environment file already configured for it, as well as its own custom package repository and binary mirror. All of these things are preconfigured by the environment and you don't need to do anything special to use them.

Now once everything is installed, within the shell in which `spack env activate` was executed, just run your kwiver build as normal, and everything should be discoverable and usable.
