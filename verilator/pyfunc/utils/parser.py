import math
import numpy as np


class BridgeParser:
    def __init__(self, array_in, bitnum_in, shape_out, bitnum_out):
        self.bit_num_in = bitnum_in
        self.bit_num_out = bitnum_out
        self.array_in = np.array(array_in, np.uint8)
        self.shape_out = shape_out

    def get_in(self, raw_array=None, bit_length=None, sign_flag=False):
        if raw_array is not None:
            arr = np.array(raw_array, np.uint8)
        elif np.size(self.array_in) > 0:
            arr = self.array_in

        if bit_length is None:
            bit_length = self.bit_num_in
        shape_in = arr.shape
        round_flag = True

        if 0 < bit_length <= 8:
            dfmt = "<B"
        elif 8 < bit_length <= 16:
            dfmt = "<H"
        elif 24 < bit_length <= 32:
            dfmt = "<u4"
        elif 56 < bit_length <= 64:
            dfmt = "<u8"
        else:
            round_flag = False
            itemsize = int(math.ceil(bit_length / 8))
            tmp = arr.reshape(-1, itemsize)
            ret = np.array([int.from_bytes(tmp[i], byteorder='little', signed=sign_flag) for i in range(0, len(tmp))]).reshape(*shape_in[:-1])

        if round_flag:
            ret = arr.view(dfmt)

        return ret.squeeze()

    def set_out(self, ret_array, bit_length=None):
        if bit_length is None:
            bit_length = self.bit_num_out
        shape_out = self.shape_out
        itemsize = int(math.ceil(bit_length / 8))
        
        ret = ret_array.reshape(-1, 1).view(np.uint8)[:, 0:itemsize].reshape(*shape_out, itemsize)
        return ret.tolist()

