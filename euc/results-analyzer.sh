#!/bin/bash

if [[ -d "$1" ]]
then results_dir="$1"
else echo "Error: results dir (first argument) must be directory. Got $1"
fi

if [[ -f "$2" ]]
then $analyzer="$2"
else echo "Error: analyzer source (second argument) must be file. Got $2"
fi

shift
shift

scratchdir=/tmp/myriastore_results_analysis_dir/

rm $scratchdir/*
rmdir $scratchdir
mkdir -p $scratchdir

for mode in USE_STRONG NO_USE_STRONG;
do for i in 1 2 3 4 5 6 7 8 9 10;
   do
	   fname=$scratchdir/"output_$mode"_"$i".hpp
	   echo '#include <string>' > $fname
	   echo '#include <vector>' >> $fname
	   cat MyriaStore/"$i"per/"$mode"/*/* | grep 'struct head' | head -1 >> $fname
	   echo "auto runs_""$mode"_"$i""() return std::vector<log> {{"
	   cat MyriaStore/"$i"per/"$mode"/*/* | grep '\[\]' | sed -e "$ ! s/\$/,/g" >>$fname
	   echo '}};}' >> $fname
   done
done

cat "$scratchdir"/* "$analyzer" | g++ --std=c++14 -o analyzer_bin -x c++ -
./analyzer-bin $*

