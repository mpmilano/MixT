#!/bin/bash

oldwd=`pwd`
cd ~/euc/

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
elif [[ -z "$strong_percentages" ]]
then
	echo "failure: specify $strong_percentages"
	exit 1
elif [[ -z "$write_percentages" ]]
then
	echo "failure: specify $write_percentages as environment variable."
	exit 1
fi


for strong_percent in $strong_percentages; do
	for write_percent in $write_percentages; do 
		first_iter=true;
		for configuration in $configurations
		do
			i=0
			echo $configuration
			exit 0
			for foo in $instance_list
			do
				i=$[i%4 + 1]
				/bin/bash test-things-loop-body.sh $i $foo $configuration $write_percent $strong_percent $strong_target $causal_target_1 $causal_target_2 $first_iter&
			done
			wait
			unset first_iter
			i=0
			echo "all done"
			for foo in $instance_list
			do
				i=$[i%4 + 1]
				/bin/bash test-things-loop-body2.sh $i $foo $configuration $strong_percent $write_percent&
			done
			wait
			echo copy done
		done
	done
done

cd $oldwd
