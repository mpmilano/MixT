#!/bin/bash

file=$1
offset=$2

granularity=1000
lines=`wc -l $file | awk '{print $1}'`
echo $lines
echo $granularity
iters=$[lines / granularity]
uselines=$[offset * granularity]
cat  $file | tail -$uselines | sort -n > "$file"-sorted
for ((i=0; i < $offset; i=$i+1)); do
		start=`head -$[$granularity + ($i * $granularity)] "$file"-sorted | tail -$granularity | head -1 | awk '{print $1}' | tr -d ','`
		end=`head -$[$granularity + ($i * $granularity)] "$file"-sorted | tail -$granularity | tail -1 | awk '{print $2}' | tr -d ','`
		echo $[ ($granularity * 1000000) / (end-start)]
done
