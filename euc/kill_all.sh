#!/bin/bash

if [[ -z "$instance_list" ]]
then
	echo "failure: specify instance_list as environment variable"
	exit 1
fi

for foo in $instance_list; do
	ssh -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@$foo sudo killall -9 vm &
done
wait
