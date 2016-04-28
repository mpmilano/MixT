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
elif [[ -z "$configurations" ]]
then
	echo "failure: specify configurations as environment variable. at least one of NORMAL, NO_USE_STRONG, NO_USE_CAUSAL must be defined here."
	exit 1
fi

first_iter=true;
for configuration in $configurations
do
	i=0
	echo $configuration
	for foo in $instance_list
	do
		i=$[i%4 + 1]
		/bin/bash test-things-loop-body.sh $i $foo $configuration $strong_target $causal_target_1 $causal_target_2 $first_iter&
	done
	wait
	unset first_iter
	i=0
	echo "all done"
	for foo in $instance_list
	do
		i=$[i%4 + 1]
		/bin/bash test-things-loop-body2.sh $i $foo $configuration&
	done
	wait
	echo copy done
done

