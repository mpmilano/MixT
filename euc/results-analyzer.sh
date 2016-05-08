#!/bin/bash

if [[ -d "$1" ]]
then results_dir="$1"
else echo "Error: results dir (first argument) must be directory. Got $1"
	 exit 1
fi

if [[ -d "$2" ]]
then analyzer="$2"
else echo "Error: analyzer directory (second argument) must be directory. Got $2"
	 exit 1
fi

if [[ -d "$3" ]]
then mutils="$3"
else echo "Error: mutils library (third argument) must be directory. Got $3"
	 exit 1
fi

shift
shift
shift

test_dir=`echo "$results_dir"/output_*o | rev | cut -d'/' -f1 | rev`
if [[ -f `find "$results_dir" -name $test_dir -type f` ]]
then echo "object files found"
else bash "$analyzer"/build-analyzer.sh $results_dir "$mutils"
fi

rm /tmp/myriastore_results_analysis_dir2/analyzer_bin
mkdir -p /tmp/myriastore_results_analysis_dir2/
clang++ -g -O3 -ferror-limit=1 -I"$mutils" -L"$mutils" --std=c++14 -lmutils -lgc -lprofiler -o /tmp/myriastore_results_analysis_dir2/analyzer_bin -I"$results_dir"  $results_dir/output*.o "$analyzer"/results-analyzer.cpp;
LD_LIBRARY_PATH="$mutils" /tmp/myriastore_results_analysis_dir2/analyzer_bin -ferror-limit=1

