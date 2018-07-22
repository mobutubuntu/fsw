# Script to run on flight Udoo x86, accompanies onboard 
# Arduino program. Uses multithreading to continuously read
# incoming serial data from COM3 and concurrently write 
# to disk. The multithreaded approach should eliminate
# pileup of unread serial data. Also attempts to synchronize
# Arduino-generated data timestaps with Udoo clock.
#
# Begin execution after start of Arduino program.
#
# Command line usage: python read_gps.py <dest directory> <port> <baudrate>
#
# Author: David Schurman
# Last Modified: 7/16/18

import sys
import time
import serial
import threading
import numpy as np
import subprocess
import signal
import os

# read initial status messages, listen for call to sync
def read_init(ser, fp, start_main, resonon_int, ocean_int):
    start_flag = False
    print('Listening for GPIO signal...')
    while not(start_flag):
        ln = ser.readline().decode('ascii')
        print(ln)
        if 'GPIO' in ln:
            start_flag = True
            # start everything
            if start_main:
                pid = os.getpid()
                th = threading.Thread(target=start_all, args=(pid, resonon_int, ocean_int))
                th.start()

    flag = False
    hdr = ''
    ser.write(b'start')
    while not(flag):
        ln = ser.readline().decode('ascii')
        hdr = (hdr + str(time.time()) + ',' + ln + '\n')
        if 'DONE SYNC' in ln:
            flag = True
            th = threading.Thread(target=write_hdr, args=(hdr, fp))
            th.start()
            # t = str(round(time.time()))
            # # once Arduino is ready to sync, write current time
            # ser.write(t.encode())
            # print("Now: " + t)
    read_data(ser, fp, callback=msg_parsed)

# continuous read loop, spawns write threads
def read_data(ser, fp, buf=b'', callback=None):
    if callback is None:
        callback = print

    max_buflen = 10000  # change to adjust size of output files
    count = 0
    cur_buflen = 0

    while True:
        waiting = ser.in_waiting
        buf = buf + ser.read(waiting)
        cur_buflen = cur_buflen + waiting
        if cur_buflen >= max_buflen:
            # spawn write thread, reset buffer
            count = count + 1
            th = threading.Thread(target=callback, args=(buf, fp, count))
            th.start()  
            buf = b''
            cur_buflen = 0

def start_all(parent_pid, resonon_int, ocean_int):
    # t_start = time.time()
    print("Starting all ...")
    args = ['python', '.\\main.py', resonon_int, ocean_int]
    proc = subprocess.Popen(args, shell=False)
    print("Started process 'main.py' with pid", proc.pid)
    time.sleep(30)
    print("Trying to end processes ...")
    # proc.send_signal(signal.SIGTERM)
    os.kill(proc.pid, signal.CTRL_C_EVENT)
    os.kill(parent_pid, signal.SIGTERM)
    return

def msg_parsed(buf, fp, fn):
    # Do something with the parsed data
    date = time.strftime("%d_%b_%Y_", time.localtime())
    b = np.array(list(buf))
    b[b == 255] = 35
    with open(fp + date + str(fn) + '.txt', 'w') as f:
        f.write(''.join(chr(i) for i in b))
    return

def write_hdr(hdr, fp):
    date = time.strftime("%d_%b_%Y_", time.localtime())
    with open(fp + date + '_readme.txt', 'w') as f:
        f.write(hdr)
    return

def main():
    # usage: python read_gps.py <path_to_dest> COM3 115200 <main?> <resonon_int> <ocean_int>
    fp = sys.argv[1]
    port = sys.argv[2]
    baudrate = sys.argv[3]
    start_main = sys.argv[4] == 'True'
    resonon_int = sys.argv[5]
    ocean_int = sys.argv[6]
    ser = serial.Serial(port, int(baudrate))
    read_init(ser, fp, start_main, resonon_int, ocean_int)
    ser.close()

if __name__ == '__main__':
    main()