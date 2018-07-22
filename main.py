# Central flight state script for Cloud Agronomics prototype drone
# each sensor will be generated and maintained by a thread and failure will 
# be localized to that functionality 

import os, sys 
import subprocess # fuck this library 
import signal
import time

# resonon_pid = None
# video_pid = None

class Main:
    def __init__(self, resonon_path, video_path, spec_path):
        self.resonon_path = resonon_path
        self.video_path = video_path
        self.spec_path = spec_path
        self.resonon_proc = None
        self.video_proc = None
        self.spec_proc = None

    def resonon_init(self) : 
        # this one really has the most overhead and will rt
        print('... starting resonon PikaXC2 spectrometer')
        # resonon_path = '.\resonon\x64\Release\spec_read.exe'
        self.resonon_proc = subprocess.Popen(self.resonon_path, shell=False)
        # resonon_pid = self.resonon_proc.pid
        print("Started process 'resonon' with pid", self.resonon_proc.pid)
        # print(process_state)

    def video_init(self):
        print('... starting video capture')
        self.video_proc = subprocess.Popen(['python', self.video_path], shell=False)
        # video_pid = self.video_proc.pid
        print("Started process 'image_cap.py' with pid", self.video_proc.pid)

    def spec_init(self):
        print('... starting Ocean Optics specrometer')
        self.spec_proc = subprocess.Popen(self.spec_path, shell=False)
        print("Started process 'spec' with pid", self.spec_proc.pid)

    def start(self) : 
        # kick things off with the resonon 
        self.resonon_init()
        self.video_init()
        self.spec_init()

    def term_handler(self, signum, frame):
        print("u just got sig'd!!!! ", signum)
        # signal video process, should be handled to save video in image_cap.py
        # self.resonon_proc.send_signal(signal.SIGTERM)
        # self.video_proc.send_signal(signal.SIGTERM)
        os.kill(self.resonon_proc.pid, signal.SIGTERM)
        os.kill(self.spec_proc.pid, signal.SIGTERM)
        os.kill(self.video_proc.pid, signal.CTRL_C_EVENT)


if __name__ == "__main__" : 
    # start()
    print("Starting main.py...")
    runner = Main('.\\resonon\\x64\\Release\\spec_read.exe', '.\\image_cap.py', '.\\')
    signal.signal(signal.SIGINT, runner.term_handler)
    runner.start()
    time.sleep(60)  # THIS NEEDS TO BE GREATER THAN SLEEP TIME IN READ_GPS.PY