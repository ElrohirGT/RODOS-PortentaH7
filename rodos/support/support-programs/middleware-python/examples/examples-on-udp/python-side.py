#!/bin/python3
# Sebastian Kind 2023 Python3 Rodos-Middleware
import struct
import time

import rodosmwinterface as rodos


# we expect here the C++ struct MyTime (see topics.h)
def topicHandler(data):
    try:
        unpacked = struct.unpack("qi", data)  # qi -> int64_t ;i int32_t
        print("timeNow  = ", unpacked[0]/1000000000.0)  
        print("msgIndex = ", unpacked[1])
    except Exception as e:
        print(e)
        print(len(data))
        print(data)


positionTopic = rodos.Topic(1002)
myTimeTopic   = rodos.Topic(1003)

myTimeTopic.addSubscriber(topicHandler)

li = rodos.LinkinterfaceUDP()
gwUDP = rodos.Gateway(li)

gwUDP.forwardTopic(positionTopic)
gwUDP.run()


cnt = 0
x = y = z = 0.0
while True:
    cnt += 1
    x   += 1.0
    y   += 100.0
    z   += 1000.0

    print("________ sending ", cnt, x, y, z)
    sendMe = struct.pack("20sIddd", b"position", cnt, x, y, z)
    positionTopic.publish(sendMe)

    time.sleep(1)

