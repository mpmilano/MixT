#!/bin/bash

if [[ $1 ]]; then
	 file_suffix="$1"
else
		file_suffix=$RANDOM
fi

if [[ -z $STRONG_REMOTE_IP ]]
then
	echo "failure: specify strong target as environment variable"
	exit 1
elif [[ -z $CAUSAL_REMOTE_IP_1 || -z $CAUSAL_REMOTE_IP_2 ]]
then
	echo "failure: specify causal target as environment variable"
	exit 1
elif [[ -z "client_rate" ]]
then
		echo "failure: specify client_rate as environment variable"
		exit 1
elif [[ -z "num_clients" ]]
then
	echo "failure: specify num_clients"
	exit 1
elif [[ -z "client_increase_rate" ]]
then
	echo "failure: specify client increase rate as environment variable."
	exit 1
elif [[ -z "test_stop_time" ]]
then
	echo "failure: specify test_stop_time as environment variable."
	exit 1
elif [[ -z "percent_causal" ]]
then
	echo "failure: specify percent_causal as environment variable."
	exit 1
elif [[ -z "$percent_read" ]]
then
	echo "failure: specify percent_read as environment variable."
	exit 1
fi

./simple_txn_test $STRONG_REMOTE_IP 8876 $CAUSAL_REMOTE_IP_2 8877 $client_rate $num_clients $client_increase_rate $test_stop_time 0.01 $percent_causal $percent_read /tmp/MyriaStore-results-2-$file_suffix 60s 5
