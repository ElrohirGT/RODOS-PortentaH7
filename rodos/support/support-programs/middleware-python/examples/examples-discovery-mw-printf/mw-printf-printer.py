#!/bin/python3
# Sebastian Kind 2023 Python3 Rodos-Middleware
#import struct
import rodosmwinterface as rodos
import time

rodos.printTopicInit(enable=True)
#rodos.printTopicInit(single_terminal=True)
#rodos.printTopicInit(enable_log_file=False)

li = rodos.LinkinterfaceUART(path="/dev/rfcomm2")
gw = rodos.Gateway(li)

gw.run()

counter = 0;
while True:
    counter += 1
    time.sleep(3)

