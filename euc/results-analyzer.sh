#!/bin/bash

if [[ -d "$1" ]]
then results_dir="$1"
else echo "Error: results dir (first argument) must be directory. Got $1"
	 exit 1
fi

if [[ -f "$2" ]]
then analyzer="$2"
else echo "Error: analyzer source (second argument) must be file. Got $2"
	 exit 1
fi

shift
shift

g++ --std=c++14 -o analyzer_bin -I"$results_dir"  $results_dir/output*.o $analyzer

