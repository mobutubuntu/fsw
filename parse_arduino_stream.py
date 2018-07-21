import sys
import json


def parse(infile, outfile):
    f = open(infile, 'r')
    contents = f.read()
    f.close()
    contents = contents.splitlines()
    imu_buf = []
    imu_times = []
    gps_times = []
    gps_full = ''
    # flag = False
    tmp = ''
    for row in contents:
        if 'a/g' in row:
            ind = row.index(',')
            imu_times.append(row[:ind])
            imu_buf.append(row[ind + 1:])
        else:
            if '(GPS)' in row:
                # flag = True
                # gps_times.append(row[:row.index(' ')])
                tmp = row[:row.index(' ')]
            else:
                if '$' in row:
                    # flag = False
                    # dollar_ind = row.index('$')
                    halved = row.split('$')
                    new_row = halved[0] + '$' + halved[1]
                    gps_full = gps_full + new_row.replace('\n', '')
                    gps_times.append(tmp)
                else: 
                    gps_full = gps_full + row.replace('\n', '')
    gps_buf = gps_full.split('$')
    if gps_full[0] == '$':
        gps_buf = gps_buf[1:]
    parse_gpsbuf(gps_buf, gps_times)
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
            "gps_times": gps_times,
            "imu_data": imu_buf,
            "imu_times": imu_times
        }, jsonfile)

def parse_gpsbuf(buf, times):
    rmc_buf = []
    rmc_times = []
    for i in range(len(buf)):
        if 'RMC' in buf[i]:
            rmc_buf.append(buf[i])
            rmc_times.append(times[i])
    print(rmc_buf)
    print(rmc_times)


def main():
    infile = sys.argv[1]
    outfile = sys.argv[2]
    parse(infile, outfile)

if __name__ == '__main__':
    main()