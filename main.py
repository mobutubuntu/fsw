# Central flight state script for Cloud Agronomics prototype drone
# each sensor will be generated and maintained by a thread and failure will 
# be localized to that functionality 

import os, sys 
import subprocess # fuck this library 
import signal
import time

class Main:
    def __init__(self, resonon_path, video_path, ocean_path):
        self.resonon_path = resonon_path
        self.video_path = video_path
        self.ocean_path = ocean_path
        self.resonon_proc = None
        self.video_proc = None
        self.ocean_proc = None

    def resonon_init(self) : 
        # this one really has the most overhead and will rt
        print('... starting resonon PikaXC2 spectrometer')
        self.resonon_proc = subprocess.Popen(self.resonon_path, shell=False)
        print("Started process 'resonon' with pid", self.resonon_proc.pid)

    def video_init(self):
        print('... starting video capture')
        self.video_proc = subprocess.Popen(['python', self.video_path], shell=False)
        print("Started process 'image_cap.py' with pid", self.video_proc.pid)

    def ocean_init(self):
        print('... starting Ocean Optics specrometer')
        self.ocean_proc = subprocess.Popen(self.ocean_path, shell=False)
        print("Started process 'spec' with pid", self.ocean_proc.pid)

    def start(self) : 
        # kick things off with the resonon 
        self.resonon_init()
        self.video_init()
        self.ocean_init()

    def term_handler(self, signum, frame):
        print("u just got sig'd!!!! ", signum)
        os.kill(self.resonon_proc.pid, signal.SIGTERM)
        os.kill(self.ocean_proc.pid, signal.SIGTERM)
        os.kill(self.video_proc.pid, signal.CTRL_C_EVENT)


if __name__ == "__main__" : 
    # start()
    print("Starting main.py...")
    resonon_int = sys.argv[1]
    ocean_int = sys.argv[2]
    runner = Main('.\\resonon\\x64\\Release\\spec_read.exe ' + resonon_int, 
        '.\\image_cap.py', '.\\OmniDriver\\x64\\Release\\SpectrumTest.exe ' + ocean_int)
    signal.signal(signal.SIGINT, runner.term_handler)
    runner.start()
    time.sleep(60)  # THIS NEEDS TO BE GREATER THAN SLEEP TIME IN READ_GPS.PY