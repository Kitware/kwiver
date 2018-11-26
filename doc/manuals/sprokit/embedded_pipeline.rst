Embedded Pipeline
=================

The embedded pipeline class provides a way to instantiate and run a
sprokit pipeline within a program. The main use case for an embedded
pipeline is in cases where a program calculates or obtains the input
and needs the output for local processing. A prime example would be a
GUI that runs a pipeline and display the results.

The pipeline to be run is passed to the ``build_pipeline()`` method as a
stream. The stream can be from a file or from a string built at run
time. If the pipeline is coming from a file, it is useful to supply
the directory portion (up to but not including the last '/') to the
call so the error messages can provide the real file name and any
*relativepath* modifiers can supply an accurate prefix. If the
pipeline comes from a string stream, do not supply this parameter.

After the pipeline has been built, it is started with the ``start()``
call. Data are supplied to the pipeline via the ``send()`` method and
retrieved with the ``receive()`` method. If input data is supplied
faster than the pipeline can accept it, the ``send()`` method will
block until there is room for the new element. Similarly, the
``receive()`` method will block if there is no output available from the
pipeline.

When all the input has been supplied to the pipeline, the
``send_end_if_input()`` is used to signal this condition. This will
cause the pipeline terminate after all the supplied data has been
processed. The ``receive()`` method will return an end of data
indication after the last pipeline output has been returned.

Example
-------

Lets look at an example to see the details of using an embedded
pipeline. The following code implements a simple pipeline that just
passes data from the input to the output. The following sections will
describe sections of this example indetail.::

  #include <sprokit/pipeline_util/literal_pipeline.h>

  // SPROKIT macros can be used to create pipeline description
  std::stringstream pipeline_desc;
  pipeline_desc << SPROKIT_PROCESS( "input_adapter",  "ia" )
                << SPROKIT_PROCESS( "output_adapter", "oa" )

                << SPROKIT_CONNECT( "ia", "counter",  "oa", "out_num" )
                << SPROKIT_CONNECT( "ia", "port2",    "oa", "port3" )
                << SPROKIT_CONNECT( "ia", "port3",    "oa", "port2" )
    ;

  // create embedded pipeline
  kwiver::embedded_pipeline ep;
  ep.build_pipeline( pipeline_desc );

  // Start pipeline
  ep.start();

  for ( int i = 0; i < 10; ++i)
  {
    // Create dataset for input
    auto ds = kwiver::adapter::adapter_data_set::create();

    // Add value to be pushed to the named port
    ds.add_value( "counter", i );

    // Data values need to be supplied to all connected ports
    // (based on the previous pipeline definition)
    ds.add_value( "port2", i );
    ds.add_value( "port3", i );
    ep.send( ds ); // push into pipeline

    // Get output from pipeline
    auto rds = ep.receive();

    // get value from the output adapter
    int val = get_port_data<int>( "out_num" );
  } // end for

  ep.send_end_of_input(); // indicate end of input

  auto rds = ep.receive(); // Retrieve end of input data item.
  if ( ! ep.at_end() || ! rds.is_end_of_data() )
  {
    // This is unexpected. Both conditions should be true.
  }


The following section builds a pipeline at runtime using a set of
macros to simplify constructing the pipeline definition. The full list
of these macros and associated documentation can be found in the
documentation for the include file. ::

    1)    #include <sprokit/pipeline_util/literal_pipeline.h>

      // SPROKIT macros can be used to create pipeline description
    2)    std::stringstream pipeline_desc;
    3)    pipeline_desc << SPROKIT_PROCESS( "input_adapter",  "ia" )
    4)                  << SPROKIT_PROCESS( "output_adapter", "oa" )

    5)                  << SPROKIT_CONNECT( "ia", "counter",  "oa", "out_num" )
    6)                  << SPROKIT_CONNECT( "ia", "port2",    "oa", "port3" )
    7)                  << SPROKIT_CONNECT( "ia", "port3",    "oa", "port2" );

Line 1 includes the file that defines a set of macros that can be used
to pragmatically create a pipeline definition.

Line 2 defines a string stream that will contain the constructed
pipeline definition.

Line 3 defines a process of type "input_adapter" that will be
referenced as "ia" in the pipeline definition.

Line 4 defines a process of type "output_adapter" that will be
referenced as "oa" in the pipeline definition.

Line 5 connects port "counter" on process "ia" (the input adapter) to
port "out_num" on process "oa" (the output adapter).

Lines 5 and 6 make additional connections between the input and output
adapter.

The folloing section created and starts the pipeline.::

    // create embedded pipeline
    8)  kwiver::embedded_pipeline ep;
    9)  ep.build_pipeline( pipeline_desc );

    // Start pipeline
   10)  ep.start();

Line 8 creates the embedded pipeline object.

Line 9 builds the pipeline based on the supplied input stream. Errors
may be detected while building the pipeline.

Line 10 starts the pipeline running. Control returns after the pipeline
is started to allow this thread to optionally supply inputs and/or
consume outputs while the pipeline runs asynchronously.

The following code illustrates how data items are supplied to the
embedded pipeline. In this sample code, ten sets of data are sent to
the pipeline with the result being read back immediately. This is not
always that realistic for more complicated pipelines because there is
usually some latency so the output will not be available until several
inputs have been supplied.::

       for ( int i = 0; i < 10; ++i)
       {
         // Create dataset for input
   11)   auto ds = kwiver::adapter::adapter_data_set::create();

         // Add value to be pushed to the named port
   12)   ds.add_value( "counter", i );

        // Data values need to be supplied to all connected ports
        // (based on the previous pipeline definition)
   13)  ds.add_value( "port2", i );
   14)  ds.add_value( "port3", i );
   15)  ep.send( ds ); // push into pipeline

        // Get output from pipeline
   16)  auto rds = ep.receive();

        // get value from the output adapter
   17)  int val = get_port_data<int>( "out_num" );
      } // end for

Line 11 creates a new adapter data set object. An adapter_data_set
contains all inputs to the pipeline. They are collected in this object
so they can be presented to the pipeline at the same time.

Lines 12 - 14 add individual data values to the pipeline input object
(adapter_data_set). The string specified in the call must match the
port name that was used to connect to the input_adapter. The value
specified will be supplied to that port.

Line 15 sends the set of input data to the input adapter process. An
error will be thrown if there is a port connected to that process
which does not have an associated data element. An error will also be
thrown if there is a element with a name that is not connected to the
input process.

Line 17 retrieves a set of output values from the pipeline. There will
be a value for each port that is connected to the output_process.


Pipeline Inputs and Outputs
---------------------------

In order to adapt a pipeline to running in an embedded manner, the
inputs that are supplied by the program are passed to the
*input_adapter* process and the outputs from the pipeline are passed to
the *output_adapter*. The pipeline definition must specify the
connections from/to these processes.

Sets of input data elements are passed to the pipeline using a
`adapter_data_set` object. This class defines a names set of data
items where the name corresponds to the port name, as specified in the
pipeline definition. The type of the data element must be compatible
with what is expected on the port by the receiving process. The
*output_adapter* returns the named data elements in the same way.

..  doxygenclass:: kwiver::adapter::adapter_data_set
    :project: kwiver
    :members:

Polling the interface queues
----------------------------

The above example code uses `send()` and `receive()` in a loop to
supply data to the pipeline and retrieve the output. While this is a
direct approach, it will not work if there is any latency in the
pipeline. That is, if the pipeline will only produce any output after
some number of inputs are supplied. Both the `send()` and `receive()`
methods will block if they can not complete, but it is possible to
check to see if these calls will block or succeed. When ready to call
`receive()`, the `empty()` method can be called to see if there is an
*adapter_data_set* available. In the same manner, the `full()` method
can be called to see if there is space to send a *adapter_data_set*
before calling `send()`.


How to Specify Pipeline
-----------------------

Pipelines are provided to the *embedded_pipeline* object as a
stream. The most common types of streams used are file streams and
string streams. To use a file stream, the controlling program needs to
open the file and pass the stream to the embedded pipeline
object. Alternatively, the pipeline can be specified as a string
stream. The easiest way to build the pipeline definition is to use the
macros supplied in

``#include <sprokit/pipeline_util/literal_pipeline.h>``

.. doxygendefine:: SPROKIT_PROCESS
   :project: kwiver

.. doxygendefine:: SPROKIT_CONFIG
   :project: kwiver

.. doxygendefine:: SPROKIT_CONNECT
   :project: kwiver

.. doxygendefine:: SPROKIT_CONFIG_BLOCK
   :project: kwiver

There are additional macros available for more detailed control over
the pipeline definition. Refer to the full documentation for the details.

If needed, the scheduler type can be specified in the pipeline
definition as follows: ::

  std::stringstream pipeline_desc;
  pipeline_desc  << SPROKIT_CONFIG_BLOCK( "_scheduler" )
                 << SPROKIT_CONFIG( "type", scheduler_type );


Advanced Topics
===============

Overriding Input and/or Output Adapters
---------------------------------------

There are some cases where the pipeline will directly source its data
rather than get it from the controlling program. Reading data directly
from a file is one example. Similarly, there are pipelines that sink
the output data directly rather than passing it back to the
controlling program. In both of these cases, the checks for mandatory
input and output adapter processes need to be bypassed to allow the
pipeline to run. This is done by deriving a class and overriding the
`connect_input_adapter()` and/or `connect_output_adapter()` method to
just return true. The following is an example of overriding the input
adapter requirement::

  class no_src_embedded_pipeline
    : public kwiver::embedded_pipeline
  {
  public:
    no_src_embedded_pipeline() { }
    virtual ~no_src_embedded_pipeline() { }

  protected:
    virtual bool connect_input_adapter() { return true; }
   };


Modifying the Pipeline Configuration
------------------------------------

There may be a situation where some part of the pipeline configuration
must be added or modified at runtime. The `update_config()` method can
be overridden in a derived class to provide the ability to make
modifications to the pipeline config prior to building the pipeline.


Embedded Pipeline Extentions
----------------------------

Embedded pipeline extentions (EPX) can be dynamically loaded based on
the pipeline configuration. One use case for EPX is to make resources
are available before starting the pipeline. This would make sure that
there are enough resources for the pipeline to start.

The EPX is a property of the pipeline configuration and can be
specified as follows: ::

    confg _pipeline:embedded_pipeline_extention
        type = foo # specify the name of extension to load
        param = value  # optional parameters


The list of available extensions can be found by entering the
following command: ::

  plugin_explorer --fact embedded_pipeline_extension

Usually EPX are application specific so it is unlikely you will find
one that is useful.

To implement your own extension, derive a class from
kwiver::embedded_pipeline_extension and implement the virtual methods.

..  doxygenclass:: kwiver::embedded_pipeline_extension
    :project: kwiver
    :members:
