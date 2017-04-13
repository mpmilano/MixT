#!/bin/bash

file=$1
offset=$2

granularity=1000
lines=`wc -l $file | awk '{print $1}'`
echo $lines
echo $granularity
iters=$[lines / granularity]
cat $file | sort -n > "$file"-sorted
for ((i=$[1 + 0$offset]; i < $iters; i=$i+1)); do
		start=`head -$[$granularity + ($i * $granularity)] "$file"-sorted | tail -$granularity | head -1 | awk '{print $1}' | tr -d ','`
		end=`head -$[$granularity + ($i * $granularity)] "$file"-sorted | tail -$granularity | tail -1 | awk '{print $2}' | tr -d ','`
		echo $[ ($granularity * 1000000) / (end-start)]
done
