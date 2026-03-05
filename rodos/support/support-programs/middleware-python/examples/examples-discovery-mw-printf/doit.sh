#!/bin/bash
set +v

echo "run source ./../../../../../setenvs.sh"
source ./../../../../../setenvs.sh

#rodos-lib.sh discovery
rodos-executable.sh discovery  mw-printf-caller.cpp
arm-none-eabi-objcopy -O binary tst flash.bin
COMMAND="st-flash write flash.bin 0x08000000"
n=0
until [ "$n" -ge 5 ]
do
   $COMMAND && break  # substitute your command here
   n=$((n+1))
   sleep 1
done


