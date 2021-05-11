Kwiver C++ -> Python
====================

This documentation pertains primarily to developers who are implementing/altering/extending a Vital type/algo, Sprokit process, or Arrow in C++.

Ideally the python binding interfaces will correspond to their respective C++ implementations as directly as possible. In support of this, when
making an update to vital, processes, or arrows, if relevant, a corresponding change should be made in the python bindings. Instructions on how/where to
perform this task follow.

The documentation for implementing Python vital components, processes, or arrows without a corresponding C++ side is described in a different section of this documentation,
link to follow.

All python bindings use [PyBind11](https://github.com/pybind/pybind11)

As a general rule for all alterations made to C++ code that do not change the outward facing interface, no change needs to be made on the Python bindings side.
If however a signature or the intended usage of an interface changes, the corresponding python binding may require a change.
See the [PyBind11 Docs](https://pybind11.readthedocs.io/en/stable/) for guidance on writing/editing binding code.
If a change is made to a C++ type bound by the python bindings that would require a change in the binding code, that edit should be made in the same PR as the corresponding change on the C++
side. Each subset of bindings has slightly differing conventions regarding the implementation/alteration of new/edited binding code. Those conventions follow below.

When extending python bindings, if a new python package dependency will be added, the Pip installable dependency should be added to the requirements-dev.txt and the CMake/setup_python.cmake
location dedicated to adding python package installs to the setup script.


Vital
=====
Types
-----
Currently all vital/types have exposed Python bindings. If a new type is added or type altered in a way that changes its outward facing interface, the bindings require an update.

Adding binding code is fairly straightforward for the vital/types. Refer to the PyBind11 documentation linked above for specific binding implementation details, and to learn how to
write PyBind11 binding code. This code should aim to mirror the C++ type interface as closely as possible while also exposing a Pythonic interface. Templated types pose a difficultly
in terms of translation to Python as Python has no concept of generics. Thus, for each type a user can instantiate the templated vital/type over, an instance of the vital/type templated
over that type must be bound. The recommended approach to doing this, to avoid writing repetitive, massive, and unwieldy binding files, is to create generic binding code, and instantiate
said binding code with the desired type, and corresponding type name. A good example of this convention can be found in the BoundingBox vital/type binding.

Once the binding code has been written, in the CMakeLists file in the python/kwiver/vital/types directory, a call to `kwiver_add_python_library` must be made, alongside a corresponding link command, and the name of the
vital type on the python side added to the call to `kwiver_create_python_init`. There are many examples of this already existing, following those examples and following those conventions
is best practice.

Simply extending a type is a little more straightforward. If the C++ change warrants a change to the python bindings, alter the python bindings in such a way as described by the PyBind11 documentation
and that should be sufficient.

In either of the above cases, a unit test should be added to python/kwiver/vital/tests for the changed/extended bindings. The conventions established in the existing tests will provide a good example/guidance.
The python testing framework is Pytest.

Algos
-----
Most if not all of the core algorithms have exposed python bindings. If a new abstract algo is added, or the interface of an algo with existing python bindings is changed in a way that
changes the interface, an update to the bindings needs to occur. Extending or adding a new algorithm is more complicated than making an alteration to a type.

If a new abstract algo is being added to the C++ core, than a corresponding binding is necessary. Algo binding code, unlike types, is split into header and source files, with the header declaring the method
that encapsulates the binding code. Follow PyBind11 documentation to write the binding code in the body of the method definition in the source file, binding each method of the
algo's interface, or in the case of an alteration to an existing algo, change/add the required binding code to properly and fully bind the algo.

Once the binding code is added, in order to extend the algo in python, for use in a process, arrow, or concrete algo implementation, a trampoline needs to be added. This trampoline follows
pybind11's convention for creating an intermediate trampoline class to allow pure Python classes to inherit from abstract c++ classes. Examples can be found in the existing algo binding directory under /trampolines
and details about Pybind11's trampoline pattern can be found in the Pybind11 documentation.

If writing a new python algo, the newly minted source and header files need to be added to the 'kwiver_add_python_library' call in the associated CMakeLists, as does the trampoline file.
**Do not forget the trampoline file, without it, the algo will be effectively unusable by Python via inheritance**

Finally in the file python/kwiver/vital/algo/algorithm_module, the algo must be registered via a call to
``register_algorithm<qualified c++ algo name, <algo trampoline><>>(m, "<desired python name>");``
and a call to the method declared in the binding code header file.

Examples of these conventions can be found in python/kwiver/vital/algo

Like types, a unittest designed to test algo registration and ability to be subclassed by Python should be added. Follow the conventions in the python/kwiver/vital/tests/alg directory.

Sprokit
=======
The current state of Python Sprokit is also somewhat limited, with only a few processes available out of the box when building Kwiver with Python. Processes differ from the components
mentioned here. Processes themselves implemented in C++ do not require binding code, as pipelines can incorporate both python and C++ processes without issue. Adding python sprokit processes will be covered in a different section of the documentation
However if changes are being made to the Sprokit Pipelines or schedulers, these will need to be reflected in the Python bindings.
To faciliate this chage in the bindings, refer to the existing Sprokit core bindings as well as the workflow for the types listed above, as the process is nearly identical.

Arrows
======
The current state of the python arrows is currently limited to a few core arrows and serialization protocols.
When adding or extending arrows, a nearly identical process to the arrow C++ -> Python algo as described above should be followed with a few small exceptions.

The primary difference between adding/editing an arrow or algo is that arrows do not require a trampoline, or a registration call. Instead, much like algo a single call to the function
encapsulating the binding code in python/kwiver/arrows/core/core_module.cxx should be made, and the files added to the appropriate lists in the related CMakeLists.
