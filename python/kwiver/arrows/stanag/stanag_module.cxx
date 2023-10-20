#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <python/kwiver/arrows/stanag/segments/stanag_4607_dwell_segment.h>
#include <python/kwiver/arrows/stanag/segments/stanag_4607_mission_segment.h>
#include <python/kwiver/arrows/stanag/segments/stanag_4607_segments.h>
#include <python/kwiver/arrows/stanag/stanag_4607_packet.h>

namespace py = pybind11;

PYBIND11_MODULE( stanag, m )
{
  stanag_4607_packet( m );
  stanag_4607_segments( m );
  stanag_4607_mission_segment( m );
  stanag_4607_dwell_segment( m );
}
