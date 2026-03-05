#!/bin/bash

source ../preliminary-checks.sh

# export PYTHONPATH="${RODOS_SUPPORT_PROGS}/middleware-python"  # already in ../preliminary-checks.sh

cat << EOT

   3 windowns shall appear, but be aware, they can be on the top of each other
   try moving the new windows

EOT


rodos-executable.sh linux-x86  *.cpp

xterm -bg white  -fg black -title rodos-printf-caller -e tst >/dev/null 2>&1  &

./mw-printf-printer.py
