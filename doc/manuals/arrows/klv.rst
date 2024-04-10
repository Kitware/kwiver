KLV
===

The KLV (Key-Length-Value) arrow provides decoding and encoding capabilities for
many common `MISB standards <https://nsgreg.nga.mil/misb.jsp>`_. This arrow does
not handle the muxing or demuxing of KLV streams in specific media formats; that
is handled in the ffmpeg arrow.

The guiding principles for this implementation are as follows:

* **Compliance.** The KLV arrow attempts to comply with the MISB standards.

* **Completeness.** The KLV arrow attempts to handle all tags, corner cases,
  and obscure features in any standard it implements.

* **Robustness.** The KLV arrow should not crash when erroneous data is
  encountered, but instead attempt to parse as much as it can. Any sections of
  unparseable KLV data are still retained in the form of raw bytes.

* **Editability.** Incoming KLV data is decoded into an intermediate software
  representation composed of standard C++ types and custom classes, allowing it
  to be manipulated independently of the encode / decode process.

* **Losslessness.** Any valid data decoded by the KLV arrow can be re-encoded in an
  equivalent form, with "equivalent" indicating no useful information is lost,
  not byte-for-byte equality.

* **Efficiency.** Encoded KLV data is quite small compared to a typical accompanying
  video stream. The KLV arrow attempts to keep the compute and memory cost of
  processing KLV to negligable levels.

* **Recency.** The KLV arrow keeps up to date with the latest versions of the MISB
  standards and does not attempt to offer perfect backwards-compatibility or
  compliance with outdated versions. However, support for some deprecated
  features may be kept around if enough data that uses those features exists in
  the wild, or if doing so is trivial.


How to Use
----------

The easiest way to leverage the capabilities of the KLV arrow is through the
``dump-klv`` command-line applet. The default printed output and ``csv``
exporter give an interpreted summary of commonly useful metadata fields
(latitude/longitude, etc.) extracted mostly from ST0601, while the ``klv-json``
exporter will give a complete tag-by-tag report across all standards.

To manipulate KLV programmatically, parsed ``klv_packet``\ s may be obtained
from an ``ffmpeg_video_input`` through the ``frame_metadata()`` method. Each of
the returned ``vital::metadata`` objects which successfully ``dynamic_cast``\ s
to ``klv_metadata`` contains one frame of ``klv_packet``\ s from one KLV stream
in the source media.


Algorithms
----------

Apply Child KLV Algorithm
-------------------------
..  doxygenclass:: kwiver::arrows::klv::apply_child_klv
    :project: kwiver
    :members:


Update KLV Algorithm
--------------------
..  doxygenclass:: kwiver::arrows::klv::update_klv
    :project: kwiver
    :members:
