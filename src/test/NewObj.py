from m5.params import *
from m5.SimObject import SimObject

class NewObj(SimObject):
    type = 'NewObj'
    cxx_header = "test/new_obj.hh"
    cxx_class = "gem5::NewObj"
    param_1 = Param.Int(1, "param_1")

