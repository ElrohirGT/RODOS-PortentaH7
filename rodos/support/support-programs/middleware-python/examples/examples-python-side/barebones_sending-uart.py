#!/bin/python3


# bare bones example sending data
# make sure to install the library first

import time
import struct

import rodosmwinterface as rodos


rodos.printTopicInit(enable=True)

luart = rodos.LinkinterfaceUART(path="/dev/rfcomm2")
gwUart = rodos.Gateway(luart)
gwUart.run()

linux2rodos = rodos.Topic(1002)
rodos2linux = rodos.Topic(1003)
gwUart.forwardTopic(linux2rodos)  # forward topic to another rodos device

cnt = 0
while True:
    sendMe = struct.pack("20sIddd", b"HALLO", cnt, 65, 65, 65)
    linux2rodos.publish(sendMe)

    cnt += 1
    time.sleep(1)
