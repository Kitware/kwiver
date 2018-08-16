Embedded Pipeline
=================

The embedded pipeline class provides a way to instantiate and run a
sprokit pipeline within a program. The main use case for an embedded
pipeline is in cases where a program calculates or obtains the input
and needs the output for local processing. A prime example would be a
GUI that needs to run a pipeline and display the results.

Required Processes
------------------

Endcaps


Running Pipeline Without Adapter Processes
------------------------------------------

There are some situations where an embedded pipeline will source then
input data or sink the output data. In these cases, the input or output
adapters are not needed.

override methods connect_input_adapter() and connect_output_adapter()


How to specify pipeline
-----------------------

  - reading from file
  - dynamically creating from macros



Embedded Pipeline Extentions
----------------------------

Embedded pipeline extentions (EPX) can be dynamically loaded based on
the pipeline configuration. One use case for EPX is to make resources
are available before starting the pipeline. This would make sure that
therare enough resources for the pipeline to start.

The EPX is a property of the pipeline configuration and can be
specified as follows:

confg _pipeline:embedded_pipeline_extention
    type = foo # specify the name of extension to load
    param = value  # optional parameters
