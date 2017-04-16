#!/bin/bash

if [[ $1 ]]; then
	 file_suffix="$1"
else
		file_suffix=$RANDOM
fi

echo "$STRONG_REMOTE_IP, 8876, $CAUSAL_REMOTE_IP_2, 8877, $client_rate, $num_clients, $client_increase_rate, $test_stop_time, 0.01, $percent_causal, $percent_read, /tmp/MyriaStore-results-2-$file_suffix, 60s 100" | ./simple_txn_test
