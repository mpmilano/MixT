#!/bin/bash
i=0

if [[ -z $strong_target ]]
then
	echo "failure: specify strong target as environment variable"
	exit 1
fi

for num_per in {1..100}
do
	echo "iteration " $num_per
	for configuration in USE_STRONG NO_USE_STRONG
	do
		echo $configuration
		for foo in $instance_list
		do
			i=$[i%4 + 1]
			/bin/bash test-things-loop-body.sh $i $foo $num_per $configuration $strong_target &
		done
		wait
		echo "all done"
		for foo in $instance_list
		do
			i=$[i%4 + 1]
			/bin/bash test-things-loop-body2.sh $i $foo $num_per $configuration $strong_target &
		done
		wait
		echo copy done
	done
done
