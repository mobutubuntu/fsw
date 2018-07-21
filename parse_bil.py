import sys
import struct
import numpy as np
import matplotlib.pyplot as plt

def parse_header(fp):
    pass

def parse_bil(fp):
    width = 1600
    height = 462
    depth = 100
    f = open(fp, 'r')
    contents = f.read()
    f.close()
    contents = bytes(contents, 'utf-8')
    # unpack binary data into a flat tuple z
    s = "<%dH" % (int(width * height * depth),)
    z = struct.unpack(s, contents)
    # print(type(z))
    z_array = np.asarray(z, dtype=np.uint16)
    z_array = np.reshape(z_array, (width, height, depth))
    # print(z_array[:, :, 0])
    for i in range(10):
        plt.contourf(z_array[:,:,i * 10])
        plt.show()
    # print(z[0:2000])


def main():
    header = sys.argv[1]
    bil = sys.argv[2]
    parse_header(header)
    parse_bil(bil)

if __name__ == "__main__":
    main()