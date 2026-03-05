#!/bin/bash
set -x

cat << EOT

_______________________________________________________
Warning: I am not installing anything. Please read this:

You can manually install our RODOS middleware interface to Python
globaly by calling
	pip install --user --upgrade -e .
in this directory.
Once installed, you can use it (import "rodosmwinterface") from any
directory on your computer.

But be careful: in some Python distributions it is very difficult to uninstall.

If you do not want to install it globaly (which I understand perfectly),
then you have to set the shell PYTHONPATH variable like this
	export PYTHONPATH="${RODOS_SUPPORT_PROGS}/middleware-python"
every time you want to import "rodosmwinterface".

___________________________________________________________

EOT



