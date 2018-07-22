import sys
import json


def parse(infile, outfile):
    f = open(infile, 'r')
    contents = f.read()
    f.close()
    contents = contents.splitlines()
    imu_buf = []
    imu_times = []
    gps_full = ''
    # flag = False
    tmp = ''
    for row in contents:
        if 'a/g' in row:
            ind = row.index(',')
            imu_times.append(row[:ind])
            imu_buf.append(row[ind + 1:])
        else:
            gps_full = gps_full + row.replace('\n', '')
    gps_buf = gps_full.split('$')
    if gps_full[0] == '$':
        gps_buf = gps_buf[1:]
    parse_gpsbuf(gps_buf)
    # print(imu_buf)
    # print(imu_times)
    # print(len(gps_buf))
    # print(len(gps_times))

    # WTF is this doing, you may ask? #
    ###################################
    # It's making separate lists of timestamps and data for 
    # both input streams - usage: 
    # Each time value in imu_times corresponds to the 
    # same-indexed element in imu_data.
    # Each time value in gps_times corresponds to the 
    # same-indexed element in gps_data (starting at beginning,
    # in case the stream gets cut off before receiving the
    # last value.
    with open(outfile, 'w') as jsonfile:
        json.dump({
            "gps_data": gps_buf,
            "imu_data": imu_buf,
            "imu_times": imu_times
        }, jsonfile)

def parse_gpsbuf(buf):
    rmc_buf = []
    for i in range(len(buf)):
        if 'RMC' in buf[i]:
            rmc_buf.append(buf[i])
    rmc_dic = {}
    for e in rmc_buf:
        print(e)
        data = e.split(',')
        if len(data) == 13:
            entry = {
                "status": data[2],
                "lat": data[3],
                "NS": data[4],
                "long": data[5],
                "EW": data[6],
                "spd": data[7],
                "cog": data[8],
                "date": data[9],
                "mv": data[10],
                "posMode": data[11],
                "navStatus*cs": data[12],
            }
            rmc_dic[float(data[1])] = entry
    print(rmc_dic)


def main():
    infile = sys.argv[1]
    outfile = sys.argv[2]
    parse(infile, outfile)

if __name__ == '__main__':
    main()