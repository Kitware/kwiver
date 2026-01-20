==========
fuse-depth
==========

The fuse-depth tool fuses depth maps from multiple cameras into a single
surface.

The default configuration of this tool depends on algorithm implementations in
the :ref:`arrows_cuda` and :ref:`arrows_vtk` arrows which will only available
if the KWIVER_ENABLE_FFMPEG and KWIVER_ENABLE_MVG CMake flags are enabled.

.. code-block:: bash

  kwiver fuse-depth    [options] input-cameras-dir input-depths-dir

**Required arguments:**

  ``input-cameras-dir`` - name of the directory containing the krtd camera
  files(default: results/krtd)

  ``input-depths-dir`` - name of the directory to read depth maps from
  (default: results/depths)


**Options are:**

  ``-h, --help``
    Display applet usage information.

  ``-c, --config arg``
    Configuration file for tool.

  ``-o, --output-config arg``
    Output a configuration. This may be seeded with a configuration file from -c/--config.

  ``-l, --input-landmarks-file arg``
    3D sparse features (default: results/landmarks.ply)

  ``-g, --input-local-space-file arg``
    Input geographic local space file (default: results/local_space.txt)

  ``-m, --output-mesh-file arg``
    Write out isocontour mesh to file (default: results/mesh.vtp)

  ``-v, --output-volume-file arg``
    Write out integrated integrated depth data to file (default: results/volume.vti)

  ``-t, --isosurface-threshold arg``
    isosurface extraction threshold (default: 0.000000).

**Default configuration**

.. literalinclude:: ../../../config/applets/fuse_depth.conf
