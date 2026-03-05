#!/bin/python3
# Sebastian Kind 2023 Python3 Rodos-Middleware
import struct
import rodosmwinterface as rodos
import time


rodos.printTopicInit(enable=True)

def topicHandler(data):
    try:
        unpacked = struct.unpack("qI", data)
        print("timeNow = ", unpacked[0])  # <-- print values that were sent from c++ rodos
        print("msgIndex = ", unpacked[1])
    except Exception as e:
        print(e)
        print(len(data))
        print(data)


linux2rodos = rodos.Topic(1002)
rodos2linux = rodos.Topic(1003)

rodos2linux.addSubscriber(topicHandler)

li = rodos.LinkinterfaceUDP()
gwUDP = rodos.Gateway(li)

gwUDP.forwardTopic(linux2rodos)
gwUDP.run()

cnt = 0
while True:

    sendMe = struct.pack("20sIddd", b"HALLO", cnt, 65, 65, 65)
    cnt+=1
    linux2rodos.publish(sendMe)

    print("publish main loop")
    time.sleep(1)

time.sleep(1000)
