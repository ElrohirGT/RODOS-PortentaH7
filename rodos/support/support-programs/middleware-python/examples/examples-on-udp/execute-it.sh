#!/bin/bash
# Author: Sebastian Kind, December 2023
# This script runs a quick example program for the middleware-python-interface


source ../preliminary-checks.sh

# export PYTHONPATH="${RODOS_SUPPORT_PROGS}/middleware-python"  ## already in ../preliminary-checks.sh

cat << EOT
  
   3 windowns shall appear, but be aware, they can be on the top of each other
   try moving the new windows

EOT



rodos-executable.sh linux-x86  rodos-side.cpp
xterm -bg white -fg black -title c++rodos-sender+receiver -e tst >/dev/null 2>&1  &

python-side.py || killall -9 tst

wait 
