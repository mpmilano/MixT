#!/bin/bash

#assume this tells me where to restart
if [[ $1 ]]; then
		start_at="$1"
else start_at=0
fi

full_range=".01 .05 .1 .15 .2 .25 .3 .35 .4 .45 .5 .55 .6 .65 .7 .75 .8 .85 .9 .99"

short_sweep=".05 .3 .7 .95"

for percent_causal in $full_range; do
		if [[ `echo "$percent_causal >= $start_at" | bc` = 1 ]]
		then for percent_read in $short_sweep; do
						 echo "starting $percent_causal, $percent_read"
						 ssh research@research.xelserv.com killall strong_relay
						 ssh research@milano.cs.cornell.edu killall causal_relay
						 ./vacuum.sh research.xelserv.com
						 ./vacuum.sh milano.cs.cornell.edu
						 export percent_causal
						 export percent_read
						 ./vm writesweep-$percent_causal-$percent_read
				 done
		fi
done
