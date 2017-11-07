#!/bin/bash

i=0
for foo in "$1"-results-iteration*; do
		./max_throughput.sh 2 30s $foo /tmp/max_throughputs/`echo $foo | rev | cut -d'/' -f1 | rev` &
		i=$[i+1];
		if [[ $i = 8 ]] ; then
				wait;
				i=0;
		fi;
done
