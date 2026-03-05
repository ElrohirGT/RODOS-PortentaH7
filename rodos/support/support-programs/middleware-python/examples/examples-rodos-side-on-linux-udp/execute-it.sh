#!/bin/bash

source ../preliminary-checks.sh

# export PYTHONPATH="${RODOS_SUPPORT_PROGS}/middleware-python" # already in ../preliminary-checks.sh

rodos-executable.sh linux-x86  *.cpp

xterm -bg white  -fg black -title rodos-printf-caller -e tst >/dev/null 2>&1  &

./UDP_example.py


