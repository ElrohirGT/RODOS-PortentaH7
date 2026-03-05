

#WARNING! Do not call this directly! 
#         it shall be soured only from the execute-it.sh scripts


#if  ! python3 -c "import rodosmwinterface" &> /dev/null 
#then 
#  echo "___________________________ rodos middleware interface in python is not installed"
#  echo "___________________________ Please execute the script 'install-python-mwinterface.sh'"
#  exit
#fi

export PYTHONPATH="${RODOS_SUPPORT_PROGS}/middleware-python"

# makes sure all terminals, subprocesses, etc are killed on exit (with trap)

intexit() {
    kill -9 -$$
}

hupexit() {
    echo
    echo "Interrupted"
    exit
}

trap hupexit HUP
trap intexit INT

