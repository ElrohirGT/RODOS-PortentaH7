#!/bin/python3

# rodos-middleware: This file is a part of the Master Thesis of Sebastian Kind 2023
# barebones example program, that opens a connection, and loggs MW_PRINTF("...\n");
# make sure to install the library first

import time
import rodosmwinterface as rodos
rodos.printTopicInit(enable=True)

print("make sure to set the right Serial/Bluetooth path in the code")

# setup a UART or bluetooth device
luart = rodos.LinkinterfaceUART(path="/dev/rfcomm2")
gwUart = rodos.Gateway(luart)
gwUart.run()



while True:
    time.sleep(1)
