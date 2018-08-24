import sys
import struct
import numpy as np
import matplotlib.pyplot as plt

def parse_header(fp):
    pass

def parse_bil(fp, dest):
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
    # z_array = np.asarray(struct.unpack(s, contents), dtype=np.uint16)
    wh = width * height
    for i in range(depth):
        buf = struct.pack("<%dH" % wh, *z[wh * i : wh * (i + 1)])
        with open(dest + 'pbt_frame_' + str(i) + '.bil', 'wb') as bf:
            bf.write(buf)
    # z_array = np.asarray(struct.unpack(s, contents), dtype=np.uint16)
    # z_array = np.reshape(z_array, (width, height, depth))
    # for i in range(10):
    #     plt.contourf(z_array[:,:,i * 10])
    #     plt.show()


def main():
    header = sys.argv[1]
    bil = sys.argv[2]
    dest = sys.argv[3]
    parse_header(header)
    parse_bil(bil, dest)

if __name__ == "__main__":
    main()