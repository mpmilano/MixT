#!/bin/bash

#assume this tells me where to restart
if [[ $1 ]]; then
		start_at="$1"
else start_at=0
fi

full_range=".01 .05 .15 .25 .35 .45 .55 .65 .75 .85 .9 .99"

short_sweep=".7"

for iteration_number in 2; do
for percent_causal in $full_range; do
		if [[ `echo "$percent_causal >= $start_at" | bc` = 1 ]]
		then for percent_read in $short_sweep; do
						 echo "starting $percent_causal, $percent_read"
						 ssh research@research.xelserv.com killall strong_relay_tracked
						 ssh research@milano.cs.cornell.edu killall causal_relay_tracked
						 sleep 15
						 echo ./simple_txn_test_tracked 128.253.3.197 8876 128.84.217.139 8877 10_Hz 510 10_Hz 10min 0.01 $percent_causal $percent_read /tmp/MyriaStore-tracked-results-iteration"$iteration_number"-causal"$percent_causal"-read"$percent_read" 5s 10 8
						 ./simple_txn_test_tracked 128.253.3.197 8876 128.84.217.139 8877 10_Hz 510 10_Hz 10min 0.01 $percent_causal $percent_read /tmp/MyriaStore-tracked-results-iteration"$iteration_number"-causal"$percent_causal"-read"$percent_read" 5s 10 8
						 
				 done
		fi
done
done
