#!/usr/bin/env bash

# Configuration
SERIAL_PORT="/dev/ttyUSB0" # Replace with your port
BAUD_RATE=115200           # Match sender's baud rate
LOG_FILE="serial_data.log" # Optional: Log data to file

screen -L -Logfile $LOG_FILE $SERIAL_PORT $BAUD_RATE

#
# # Exit on errors and clean up
# trap 'stty sane; exit 0' SIGINT SIGTERM # Restore port on Ctrl+C/exit
#
# # Configure the serial port
# stty -F "$SERIAL_PORT" "$BAUD_RATE" cs8 -cstopb -parenb -ixon -ixoff -icanon min 1 time 0
#
# echo "Listening on $SERIAL_PORT (${BAUD_RATE}bps)... Press Ctrl+C to stop."
# echo "Logs saved to $LOG_FILE"
#
# # Main loop: Read and process data
# while true; do
# 	# Read raw data (blocking until 1+ bytes received)
# 	if read -r -n 1024 data <"$SERIAL_PORT"; then # Read up to 1024 bytes
# 		# Timestamp and log data
# 		timestamp=$(date +"%Y-%m-%d %H:%M:%S")
# 		echo "[$timestamp] Raw: $data" | tee -a "$LOG_FILE" # Tee to console and log
#
# 		# Add interpretation logic here (see Step 4)
# 	fi
# done
