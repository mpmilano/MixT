#!/bin/bash

accum0=0
count0=0
for number in $*; do
		accum0=$[accum0 + number]
		count0=$[count0+1]
done
accum=0
count=0
for number in $*; do
		if [[ $[number] -gt 6000 ]]
		then
				echo $number
				accum=$[accum + number]
				count=$[count+1]
		fi
done



