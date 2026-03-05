#!/bin/python3


# bare bones example sending data
# make sure to install the library first

import time
import struct

import rodosmwinterface as rodos

rodos.printTopicInit(enable=True)

def topicHandler(data):
    try:
        unpacked = struct.unpack("qI4b", data)
        print("timeNow = ", unpacked[0])
        print("msgIndex = ", unpacked[1])
    except Exception as e:
        print(e)
        print(data)


luart = rodos.LinkinterfaceUART(path="/dev/rfcomm2")
gwUart = rodos.Gateway(luart)
gwUart.run()

linux2rodos = rodos.Topic(1002)
rodos2linux = rodos.Topic(1003)

rodos2linux.addSubscriber(topicHandler) # <<<<<< register callback

cnt = 0
while True:
    cnt += 1
    time.sleep(1)
