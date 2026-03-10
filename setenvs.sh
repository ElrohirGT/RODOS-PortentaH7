#!/usr/bin/env bash
scriptName=$_

#_________________________________________Check it has to be sourced!
# Source check should work on most shells
# See this: https://stackoverflow.com/questions/2683279/how-to-detect-if-a-script-is-being-sourced
sourced=0
if [ -n "$ZSH_EVAL_CONTEXT" ]; then
  case $ZSH_EVAL_CONTEXT in *:file) sourced=1;; esac
elif [ -n "$KSH_VERSION" ]; then
  [ "$(cd $(dirname -- $0) && pwd -P)/$(basename -- $0)" != "$(cd $(dirname -- ${.sh.file}) && pwd -P)/$(basename -- ${.sh.file})" ] && sourced=1
elif [ -n "$BASH_VERSION" ]; then
  (return 0 2>/dev/null) && sourced=1
else # All other shells: examine $0 for known shell binary filenames
  # Detects `sh` and `dash`; add additional shell filenames as needed.
  case ${0##*/} in sh|dash) sourced=1;; esac
fi

if [ $sourced -eq 0 ]
then
   echo "_____________________________________________________ ERROR"
   echo "ERROR: Please call this script as source, and not as subshell"
   echo "like this:"
   echo "      %source $scriptName"
   echo "___________________________________________________________"
   exit
fi
#_____________________________________________________ Set Directories

export QTZ_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export QTZ_RODOS="$QTZ_ROOT/rodos"
export QTZ_CM4="$QTZ_ROOT/CM4"
export QTZ_CM7="$QTZ_ROOT/CM7"

export PATH=${PATH}:${QTZ_ROOT}/scripts/build-scripts

cd rodos && source ./setenvs.sh && cd ..

complete -W "cm4 cm7" qtz-lib.sh
#complete -W "cm4 cm7" qtz-executable.sh

echo "#--------------------------------------#"
echo "| Usage: All commands start with qtz-  |"
echo "#--------------------------------------#"
