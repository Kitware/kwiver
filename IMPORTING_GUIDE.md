# Importing modules from kwiver v1

This document summarizes the main steps when importing an algorithms / plugin implementations from kwiver v1.
The instructions are currently restricted to C++.

## Importing interface classes

This is the simplest case. 

* If the class inherits from
`kwiver::vital::algorithm_def<>` change it to inherit directly from
`kwiver::vital::algorithm` the templated algorithm class has been removed.
* Remove `INSTANTIATE_ALGORITHM_DEF` from the cxx file if it exists as well as the line including `algorithm.txx`
* Make the default constructor public if it isn't 
* Remove the function with signature
```
static std::string static_type_name() { return <name>; }
```
* Add
```
PLUGGABLE_INTERFACE(class_name);
```
in the declaration of the class.

For reference compare `vital/algo/video_input.h` in this repository and in kwiver v1

## Importing algorithm implementation classes

* Remove The `PLUGIN_INFO` block from header file
* Remove the following functions declarations if they exist:
  * constructor
  * `virtual vital::config_block_sptr get_configuration() const;`
  * `virtual void set_configuration(vital::config_block_sptr config);`
* If the constructor is not empty:
  * move the logic inside a private member function declared as `void initialize() ovverride;`
  * If the PIMPL idiom is used declare the pointer using the `KWIVER_UNIQUE_PTR` macro. For example:
  ```
  class priv;
  const std::unique_ptr<priv> d;
  ```
  becomes 
  ```
  class priv;
  KWIVER_UNIQUE_PTR(priv,d);
  ```
  Then inside the implementation of `initialize()` use `KWIVER_INITIALIZE_UNIQUE_PTR(priv,d); `

* If `set_configuration` has additional logic besides setting parameters move this inside a private member function declared as `void set_configuration_internal() override`
* Each algorithm implementation should contain a block defined with the `PLUGGABLE_IMPL` macro:
  ```
  PLUGGABLE_IMPL(
  <class name>,
  <description>,
  < list of PARAM/PARAM_DEFAULT macros)
  ```
  where
  ```
  PARAM_DEFAULT(
  <parameter name>,
  <parameter type>,
  <parameter description>,
  <default value>)
  ```
  and 
  ```
  PARAM(
  <parameter name>,
  <parameter type>,
  <parameter description>)
  ```
  * `class name` is the name of the current class
  * `description` should be copied from `PLUGIN_INFO`
  * `PARAM` list should be populated based on the content of `get_configuration()` in the cxx file

* Finally, since the parameters are now proper member variables of the class the PIMPL pointer needs to hold references to the parent values instead of holding its  own. So, convert the members of the PIMPL class to functions and update the calling code. 

For a complete example compare `arrows/core/video_input_filter.{cxx,h}` between kwiver-v2 and kwiver-v1 . For a more complex  example check
`arrows/ffmpeg/ffmpeg_video_output.{cxx,h}` .

## Importing applets
This is similar to algorithms . See `arrows/core/applets/dump_klv.{cxx,h}` or `arrows/klv/applets/compare_klv.{cxx,h}`