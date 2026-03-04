======================
init-cameras-landmarks
======================

The init-cameras-landmarks tool estimate cameras and landmarks from a set of
feature tracks.

The default configuration of this tool depends on algorithm implementations in
the :ref:`arrows_ffmpeg`, :ref:`arrows_mvg`, and :ref:`arrows_opencv` arrows
which will only available if the KWIVER_ENABLE_FFMPEG, KWIVER_ENABLE_MVG, and
KWIVER_ENABLE_OPENCV CMake flags are enabled.

.. code-block:: bash

  kwiver init-cameras-landmarks       [options]

**Options are:**

  ``-h, --help``
    Display applet usage information.

  ``-c, --config arg``
    Configuration file for tool

  ``-o, --output-config arg``
    Output a configuration. This may be seeded with a configuration file from -c/--config.

  ``-v, --video arg``
    Input video

  ``-t, --tracks arg``
    Input tracks

  ``-k, --camera arg``
    Output directory for cameras

  ``-l, --landmarks arg``
    Output landmarks file

  ``-g, --local-space arg``
    Output geographic local space file

**Default configuration**

.. literalinclude:: ../../../config/applets/track_features.conf
