# This file is part of KWIVER, and is distributed under the
# OSI-approved BSD 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

import numpy

from kwiver.sprokit.processes.kwiver_process import KwiverProcess
from kwiver.sprokit.pipeline import datum
from kwiver.sprokit.pipeline import process
import kwiver.vital.types as kvt

class HomographyReaderProcess(KwiverProcess):
    def __init__(self, config):
        KwiverProcess.__init__(self, config)

        self.add_config_trait('input', 'input', '', 'Path to input file')
        self.declare_config_using_trait('input')

        optional = process.PortFlags()
        self.declare_output_port_using_trait('homography_src_to_ref', optional)

    def _configure(self):
        self.fin = open(self.config_value('input'))
        self._base_configure()

    def _step(self):
        line = self.fin.readline()
        if not line:
            self.mark_process_as_complete()
            d = datum.complete()
            self.push_datum_to_port_using_trait('homography_src_to_ref', d)
            return
        line = line.split()
        array, (from_id, to_id) = line[:9], line[9:]
        array = numpy.array(list(map(float, array))).reshape((3, 3))
        homog = kvt.F2FHomography.from_doubles(array, int(from_id), int(to_id))
        self.push_to_port_using_trait('homography_src_to_ref', homog)
        self._base_step()

def __sprokit_register__():
    from kwiver.sprokit.pipeline import process_factory
    module_name = 'python:kwiver.read_homography'
    if process_factory.is_process_module_loaded(module_name):
        return
    process_factory.add_process('kw_read_homography',
                                'A Simple Kwiver homography reader',
                                HomographyReaderProcess)
    process_factory.mark_process_module_as_loaded(module_name)

