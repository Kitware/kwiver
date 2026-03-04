==================
bundle-adjust-tool
==================

The bundle-adjust-tool optimizes cameras and landmarks via a bundle adjustment
algorithm.

The default configuration of this tool depends on algorithm implementations in
the :ref:`arrows_ceres` and :ref:`arrows_mvg` arrows which will only be
available if the KWIVER_ENABLE_CERES and KWIVER_ENABLE_MVG CMake flags are
enabled.

.. code-block:: bash

  kwiver bundle-adjust-tool    [options]

**Options are:**

  ``-h, --help``
    Display applet usage information.

  ``-c, --config arg``
    Configuration file for tool.

  ``-p, --GCP arg``
    Input 3D Ground Control Points (GCP) with corresponding 2D
    Camera Registration Points (CRP) as JSON file.

  ``-v, --video arg``
    Input video file or image.txt list.

  ``-t, --tracks arg``
    Input tracks.txt

  ``-i, --cam_in arg``
    Input camera models.txt list.

  ``-k, --cam_out arg``
    Output directory for camera models.

  ``-l, --landmarks arg``
    Output landmarks.ply file.

  ``-g, --local-space arg``
    Output geographic local space file.

  ``-o, --output-config arg``
    Output a configuration, which may be seeded with a configuration file
    from -c/--config.

**Default configuration**

.. literalinclude:: ../../../config/applets/bundle_adjust_tool.conf
