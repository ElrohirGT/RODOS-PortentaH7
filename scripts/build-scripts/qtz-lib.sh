#!/usr/bin/env bash

set -e

DIR_NAME=$(dirname "$0")
source "$DIR_NAME"/../quit-if-environment-is-wrong.sh

if [ "$1" != "cm4" ] && [ "$1" != "cm7" ]; then
	echo "ERROR: Invalid argument received!"
	echo "- cm4"
	echo "- cm7"
	echo "Usage: qtz-lib.sh <arch>"
	exit
fi

cd ./rodos

echo "#---------------------------------#"
echo "|               $1               |"
echo "#---------------------------------#"
rodos-lib.sh "$1"

cd ..
