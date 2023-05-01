import math
import numpy as np
from .utils.parser import BridgeParser


def callback(array_in, bitnum_in, shape_out, bitnum_out):
    print("[Python] array_in = " + str(array_in) + " ; bit_num(in) = " + str(bitnum_in))
    print("[Python] shape_out = " + str(shape_out) + " ; bit_num(out) = " + str(bitnum_out))
    print("*" * 20)

    parser = BridgeParser(array_in, bitnum_in, shape_out, bitnum_out)

    dat_in = parser.get_in(sign_flag=False)
    print("[view] dat_in = ", dat_in)
    print("[info] dat_in.shape = ", dat_in.shape)

    dat_in += 1
    
    dat_out = parser.set_out(dat_in)
    print("[view] dat_out = ", dat_out)
    print("[info] len(dat_out) = ", len(dat_out))
    return dat_out
