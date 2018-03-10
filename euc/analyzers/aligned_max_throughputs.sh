#!/bin/bash

cd /tmp/max_throughputs/
for bar in *iteration2*read.7; do
		suffix=`echo $bar | sed s/.*iteration.//g | rev | cut -d'/' -f1 | rev`;
		for foo in ignore; do
				pre_num=`echo $suffix | cut -d'.' -f2 | cut -d'-' -f1`;
				echo $pre_num | sed s/^.$/"$pre_num"0/g; echo '|';
				cat *"$suffix" | xargs ~/research/andrew/consistency-tester/euc/analyzers/average.sh;
		done | tr "\n" "," | cut -d',' -f1-10 ;
done | sort -n | sed s/,/', '/g | sort -n | sed s/'|,'/'|'/g
