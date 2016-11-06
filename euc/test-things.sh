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
	echo "failure: specify strong_percentages"
	exit 1
elif [[ -z "$write_percentages" ]]
then
	echo "failure: specify write_percentages as environment variable."
	exit 1
elif [[ -z "$starting_rate" ]]
then
	echo "failure: specify starting_rate as environment variable."
	exit 1
elif [[ -z "$increase_by" ]]
then
	echo "failure: specify increase_by as environment variable."
	exit 1
elif [[ -z "$increase_delay" ]]
then
	echo "failure: specify increase_delay as environment variable."
	exit 1
elif [[ -z "$test_stop_time" ]]
then
	echo "failure: specify test_stop_time as environment variable."
	exit 1
elif [[ -z "$max_threads" ]]
then
	echo "failure: specify max_threads as environment variable."
	exit 1
elif [[ -z "$ndebug" ]]
then
	echo "failure: specify ndebug (either spaceful string or -DNDEBUG) as environment variable."
	exit 1
fi

echo "$increase_by"

for write_percent in $write_percentages; do 
	for strong_percent in $strong_percentages; do
		first_iter=true;
		for configuration in $configurations
		do
			i=0
			echo $configuration
			for foo in $instance_list
			do
				i=$[i%4 + 1]
				/bin/bash test-things-loop-body.sh $i $foo $configuration $ndebug $write_percent $strong_percent $strong_target $causal_target_1 $causal_target_2 $starting_rate $increase_by $increase_delay $test_stop_time $max_threads $first_iter&
			done
			wait
			ssh research@"$strong_target" killall strong_receiver
			ssh research@"$causal_target_1" killall causal_receiver
			ssh research@"$causal_target_2" killall causal_receiver
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
