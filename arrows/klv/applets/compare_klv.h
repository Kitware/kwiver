// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of compare-klv applet.

#ifndef KWIVER_ARROWS_KLV_APPLETS_DUMP_KLV_H_
#define KWIVER_ARROWS_KLV_APPLETS_DUMP_KLV_H_

#include <vital/applets/kwiver_applet.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
class compare_klv
  : public kwiver::tools::kwiver_applet
{
public:
  compare_klv();

  PLUGIN_INFO(
    "compare-klv",
    "Compare two sources of KLV.\n\n"
    "This program prints differences found between the KLV in two files "
    "(video or JSON)." );

  int run() override;
  void add_command_options() override;

private:
  class impl;
  std::unique_ptr< impl > d;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
