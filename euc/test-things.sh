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
elif [[ -z "$causal_percentages" ]]
then
	echo "failure: specify causal_percentages"
	exit 1
elif [[ -z "$read_percentages" ]]
then
	echo "failure: specify read_percentages as environment variable."
	exit 1
elif [[ -z "$num_clients" ]]
then
	echo "failure: specify num_clients as environment variable."
	exit 1
elif [[ -z "$client_rate" ]]
then
	echo "failure: specify client_rate as environment variable."
	exit 1
elif [[ -z "$client_increase_rate" ]]
then
	echo "failure: specify client_increase_rate as environment variable."
	exit 1
elif [[ -z "$test_stop_time" ]]
then
	echo "failure: specify test_stop_time as environment variable."
	exit 1
elif [[ -z "$max_threads" ]]
then
	echo "failure: specify max_threads as environment variable."
	exit 1
fi
if [[ "$rebuild" = "true" ]]
then first_iter="true"
fi

for read_percent in $read_percentages; do 
	for causal_percent in $causal_percentages; do
		i=0
		ssh research@"$strong_target" killall strong_relay
		ssh research@"$causal_target_1" killall causal_relay
		ssh research@"$causal_target_2" killall causal_relay
		for foo in $instance_list
		do
				i=$[i%4 + 1]
				/bin/bash test-things-loop-body.sh $i $foo $ndebug $read_percent $causal_percent $strong_target $causal_target_1 $causal_target_2 $num_clients $client_rate $client_increase_rate $test_stop_time $max_threads $first_iter&
		done
		wait
		unset first_iter
		i=0
		echo "all done"
		for foo in $instance_list
		do
				i=$[i%4 + 1]
				/bin/bash test-things-loop-body2.sh $i $foo $causal_percent $read_percent&
		done
		wait
		echo copy done
	done
done

cd $oldwd
