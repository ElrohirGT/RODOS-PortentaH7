#!/usr/bin/env bash
if [ ! -z $QTZ_ROOT ]; then # it is ok!
	return
fi

cat <<EOT

  *****************************************************
  *** please call (only once) from repository root directory
  ***         source set-envs.sh
  *** else I can not do any thing!
  ****************************************************

EOT

exit
