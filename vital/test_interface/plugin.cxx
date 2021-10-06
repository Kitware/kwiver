// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/plugin_management/plugin_loader.h>
#include <vital/test_interface/say.h>
#include <vital/test_interface/say_cpp_export.h>


// ----------------------------------------------------------------------------
namespace kwiver::vital
{

class cpp_say_impl : public say
{
public:
  cpp_say_impl() = default;
  ~cpp_say_impl() override = default;

  std::string says() override
  {
    return "I am the C++ plugin";
  }

  static pluggable_sptr from_config( config_block const& /* cb */ )
  {
    // No parameter constructor, just return new instance
    return std::make_shared<cpp_say_impl>();
  }

  static void get_default_config( config_block & /* cb */ )
  {
    // No constructor parameters, so nothing to set in the config block.
  }
};

}


// ----------------------------------------------------------------------------
namespace kv = kwiver::vital;

extern "C"
SAY_CPP_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  vpl.add_factory<kv::say, kv::cpp_say_impl>( "cpp" );
}
