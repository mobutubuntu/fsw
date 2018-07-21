import cv2
import time
import threading
import numpy as np
import signal

def w(camera, out, times, count):
    print('times' + count + '.txt')
    camera.release()
    out.release()
    with open('times' + count + '.txt', 'w') as f:
        f.write(times)
    return


# for i in range(10):
camera = cv2.VideoCapture(0)
width = 3264
height = 2448
camera.set(3,width)
camera.set(4,height)

fourcc = cv2.VideoWriter_fourcc(*'FMP4')
out = cv2.VideoWriter('E:\\video\\out' + str(20) + '.avi',fourcc, 30.0, (width,height))

times = ''

def term_handler(signum, frame):
    print("ur vid just got sig'd!!! ", signum)
    camera.release()
    out.release()
    with open('E:\\video\\times20.txt', 'w') as f:
        f.write(times[(times.find('\n') + 1):])
    exit()

signal.signal(signal.SIGINT, term_handler)

# count = 0
t1 = time.time()
t = time.time()
while(camera.isOpened()):
    # Capture frame-by-frame
    ret, frame = camera.read()
    t = time.time()
    times = times + str(t) + '\n'
    # print(count)
    if ret == True:
        out.write(frame)
        # Display the resulting frame
        # cv2.imshow('Frame',frame)
    
        # Press Q on keyboard to  exit
        # if cv2.waitKey(25) & 0xFF == ord('q'):
        #   break
                
    # Break the loop
    else: 
        break
# th = threading.Thread(target=w, args=(camera, out, times, str(i)))
# th.start()
# When everything done, release the video capture object
camera.release()
out.release()
with open('E:\\video\\times20.txt', 'w') as f:
    f.write(times[(times.find('\n') + 1):])  # remove first stamp
# cv2.destroyAllWindows()
# for i in range(10):
#     t = time.time()
#     # return_value, image = camera.read()
#     camera.grab()
#     # t2 = time.time()
#     retval, image = camera.retrieve(0)
#     th = threading.Thread(target=w, args=('img_'+ str(t) +'.png', image))
#     th.start()
#     time.sleep(1)
# del(camera)