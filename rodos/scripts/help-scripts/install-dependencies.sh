#!/usr/bin/env bash

# Raspbian is missing some non-essential packages, enable if on Raspbian to not try to install these
ON_RASPBIAN=false 

# if true, extra tools will be installed. If false, only the bare minimum to compile on posix+arm will be
NON_ESSENTIAL=false

function CheckAndInstallPackage {

	local OUTPUT="Checking for $1:"
	local PKG_OK
	
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' "$1" 2>/dev/null | grep "install ok installed") >/dev/null

	if [ "" = "$PKG_OK" ]; then
		OUTPUT="$OUTPUT \033[1;31mNOT FOUND\033[0m"
		echo -e "${OUTPUT}"
	elif [ "install ok installed" = "$PKG_OK" ]; then
		OUTPUT="$OUTPUT \033[1;32mFOUND\033[0m"
		echo -e "${OUTPUT}"
		return
	fi

	echo -e "Installing package $1"
	sudo apt-get --yes install "$1"

	echo -e "\nChecking if install was successful"
	OUTPUT="Checking for $1:"
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' "$1" 2>/dev/null |grep "install ok installed") >/dev/null

	if [ "" = "$PKG_OK" ]; then
		OUTPUT="$OUTPUT \033[1;31mNOT FOUND\033[0m"
		echo -e "${OUTPUT}"
		echo -e "\033[1;31mERROR\033[0m: Package $1 could not be installed, please try manual installation."
		exit 2
	elif [ "install ok installed" = "$PKG_OK" ]; then
		OUTPUT="$OUTPUT \033[1;32mFOUND\033[0m"
		echo -e "${OUTPUT}"
	fi
}

echo -e "\nChecking required packages\n"
CheckAndInstallPackage "apt-utils"
CheckAndInstallPackage "build-essential"
CheckAndInstallPackage "clang"
CheckAndInstallPackage "gcc-arm-none-eabi"
CheckAndInstallPackage "binutils-arm-none-eabi"
CheckAndInstallPackage "libnewlib-arm-none-eabi"

if [ "$ON_RASPBIAN" = false ]; then
if [ "$NON_ESSENTIAL" = true ]; then
    CheckAndInstallPackage "gcc-multilib"
    CheckAndInstallPackage "g++-multilib"
fi
fi


if [ "$NON_ESSENTIAL" = true ]; then
    CheckAndInstallPackage "clang-format"
    CheckAndInstallPackage "clang-tools"
    CheckAndInstallPackage "gdb"
    CheckAndInstallPackage "cmake"
fi