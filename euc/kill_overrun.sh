#!/bin/bash


if [[ `./running_time.sh $*` -gt 25 ]]; then
		killall $*
else
		echo "spared"
fi

