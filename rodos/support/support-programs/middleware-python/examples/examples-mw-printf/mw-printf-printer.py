#!/bin/python3
# Sebastian Kind 2023 Python3 Rodos-Middleware
#import struct
import rodosmwinterface as rodos
import time

#rodos.printTopicInit(enable=True)
#rodos.printTopicInit(single_terminal=False)
rodos.printTopicInit(enable_log_file=True)

li = rodos.LinkinterfaceUDP()
gwUDP = rodos.Gateway(li)
gwUDP.run()

counter = 0;
while True:
    print("Python-Loop: doing nothing ", counter)
    counter += 1
    time.sleep(3)

