// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface for detected_object_set_output_kpf

#ifndef KWIVER_ARROWS_DETECTED_OBJECT_SET_OUTPUT_KPF_H
#define KWIVER_ARROWS_DETECTED_OBJECT_SET_OUTPUT_KPF_H

#include <arrows/kpf/kwiver_algo_kpf_export.h>

#include <vital/algo/detected_object_set_output.h>

namespace kwiver {

namespace arrows {

namespace kpf {

class KWIVER_ALGO_KPF_EXPORT detected_object_set_output_kpf
  : public vital::algo::detected_object_set_output
{
public:
  PLUGGABLE_IMPL(
    detected_object_set_output_kpf,
    "Detected object set writer using kpf format."
  )

  virtual ~detected_object_set_output_kpf() = default;

  virtual bool check_configuration( vital::config_block_sptr config ) const;

  virtual void write_set(
    const kwiver::vital::detected_object_set_sptr set,
    std::string const& image_name );

private:
  void initialize() override;

  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace kpf

} // namespace arrows

}     // end namespace

#endif // KWIVER_ARROWS_DETECTED_OBJECT_SET_OUTPUT_KPF_H
