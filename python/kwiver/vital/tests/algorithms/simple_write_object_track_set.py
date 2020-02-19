# ckwg +29
# Copyright 2019 by Kitware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
#  * Neither name of Kitware, Inc. nor the names of any contributors may be used
#    to endorse or promote products derived from this software without specific
#    prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
from kwiver.vital.algo import WriteObjectTrackSet

class SimpleWriteObjectTrackSet(WriteObjectTrackSet):
    """                                                                        
    Implementation of a basic WriteObjectTrackSet. Uses
    buff_is_open to simulate opening and closing a file, and buff
    to simulate writing to a file             
    """  

    def __init__(self):
        WriteObjectTrackSet.__init__(self)
        self.threshold = 0.0
        self.buff_is_open = False
        self.buff = ""

    def get_configuration(self):
        # Inherit from the base class 
        cfg = super(WriteObjectTrackSet, self).get_configuration()
        cfg.set_value( "threshold", str(self.threshold) )
        return cfg

    def set_configuration( self, cfg_in ):
        cfg = self.get_configuration()
        cfg.merge_config(cfg_in)
        self.threshold = float(cfg.get_value("threshold"))
                                                                             
    def check_configuration( self, cfg ):
        return (not cfg.has_value("threshold") or float(cfg.get_value("threshold"))==self.threshold)
        
    def close(self):
        self.buff_is_open = False
    
    # Ignores other_file_name and writes to self.buff
    def open(self, other_file_name):
        self.buff_is_open = True

    # Just writes the size to the buffer
    def write_set(self, set):
        self.buff += str(set.size())




def __vital_algorithm_register__():                                            
    from kwiver.vital.algo import algorithm_factory
     # Register Algorithm 
    implementation_name  = "SimpleWriteObjectTrackSet"
    if algorithm_factory.has_algorithm_impl_name(SimpleWriteObjectTrackSet.static_type_name(), implementation_name):
        return
        
    algorithm_factory.add_algorithm( implementation_name, "test simple write object track set", SimpleWriteObjectTrackSet )
    algorithm_factory.mark_algorithm_as_loaded( implementation_name )