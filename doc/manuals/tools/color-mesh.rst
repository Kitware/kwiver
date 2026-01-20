==========
color-mesh
==========

The color-mesh tool colors an input-mesh from a video (or list of images) and a
list of camera files stored in a directory. A mesh colored with the
average color or with a color for a particular camera is produced.

The default configuration of this tool depends on algorithm implementations in
the :ref:`arrows_vtk` arrow which will only available if the
KWIVER_ENABLE_VTK CMake flags are enabled.

.. code-block:: bash

  kwiver color-mesh       [options] input-mesh video-file cameras-dir output-mesh

**Required arguments:**

  ``input-mesh``  - input mesh file.

  ``video-file``  - input video file.

  ``cameras-dir``  - input camera directory.

  ``output-mesh`` - output mesh file.

**Options are:**

  ``-h, --help``
    Display applet usage information.

  ``-a, --all-frames``
    Compute average color or save each frame color.

  ``-c, --config arg``
    Configuration file for tool

  ``-f, --frame arg``
    Frame index to use for coloring. If -1 use an average color
    for all frames. (default: -1)

  ``-g, --input-local-space-file arg``
    Input geographic local space file.

  ``-m, --mask-file arg``
    An input mask video or list of mask images to indicate which pixels to ignore.

  ``-o, --output-config arg``
    Output a configuration. This may be seeded with a configuration file
    from -c/--config.

  ``-v, --active-attribute arg``
    Choose the active attribute between mean, median and count when saving
    a composite color (all-frames is false). For the VTP format, all
    attributes are saved, for PLY only the active attribute is saved.

  ``-s, --frame-sampling arg``
    Use for coloring only frames that satisfy frame mod sampling == 0 (default: 1)

**Default configuration**

.. literalinclude:: ../../../config/applets/color_mesh.conf
