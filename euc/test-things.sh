#!/bin/bash
i=0
if [[ -z $strong_target ]]
then
	echo "failure: specify strong target as environment variable"
	exit 1
elif [[ -z $causal_target_1 || -z $causal_target_2 ]]
then
	echo "failure: specify causal target as environment variable"
	exit 1
elif [[ -z "$instance_list" ]]
then
	echo "failure: specify instance_list as environment variable"
	exit 1
elif [[ -z "$range_max" ]]
then
	echo "failure: specify range_max as environment variable"
	exit 1
fi

first_iter=true;
for (( num_per = 1; num_per <= range_max; num_per = num_per + 1))
do
	echo "iteration " $num_per
	for configuration in USE_STRONG NO_USE_STRONG
	do
		i=0
		echo $configuration
		for foo in $instance_list
		do
			i=$[i%4 + 1]
			/bin/bash test-things-loop-body.sh $i $foo $num_per $configuration $strong_target $causal_target_1 $causal_target_2 $first_iter&
		done
		wait
		unset first_iter
		i=0
		echo "all done"
		for foo in $instance_list
		do
			i=$[i%4 + 1]
			/bin/bash test-things-loop-body2.sh $i $foo $num_per $configuration&
		done
		wait
		echo copy done
	done
done
