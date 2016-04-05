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

scratchdir=/tmp/myriastore_results_analysis_dir/

rm $scratchdir/*
rmdir $scratchdir
mkdir -p $scratchdir


echo '#pragma once' > "$scratchdir"/header.hpp
echo '#include <string>' >> "$scratchdir"/header.hpp
echo '#include <vector>' >> "$scratchdir"/header.hpp
cat "$results_dir"/*/*/*/* | grep 'struct log' | head -1 >> "$scratchdir"/header.hpp

for mode in USE_STRONG NO_USE_STRONG;
do for i in 1 2 3 4 5 6 7 8 9 10;
   do
	   fname=$scratchdir/"output_$mode"_"$i"
	   echo "#include \"header.hpp\"" > "$fname".hpp
	   echo "std::vector<log> runs_""$mode"_"$i""();" >> "$fname".hpp
	   echo "#include \"$fname"".hpp\"" > "$fname".cpp
	   echo "std::vector<log> runs_""$mode"_"$i""() { return std::vector<log> {{" >> "$fname".cpp
	   cat "$results_dir"/"$i"per/"$mode"/*/* | grep '\[\]' | sed -e "$ ! s/\$/,/g" >>"$fname".cpp
	   echo '}};}' >> "$fname".cpp
   done
done

cat "$scratchdir"/output_*hpp "$analyzer" > "$scratchdir"/main.cpp
wd=`pwd`
cd "$scratchdir"

#build the makefile
names=""
for mode in USE_STRONG NO_USE_STRONG;
do for i in 1 2 3 4 5 6 7 8 9 10;
   do
	   names="$names output_$mode"_"$i"
   done
done

objects=""
for foo in $names;
do objects="$objects $foo".o
done

echo "all: $objects main.o" > Makefile
echo -n -e '\t' >> Makefile
echo "clang++ $objects main.o -o $wd""/analyzer_bin" >> Makefile

for name in $names
do
	echo "$name"".o:" >> Makefile
	echo -n -e '\t' >> Makefile
	echo "g++ --std=c++14 -c $name".cpp >> Makefile
done

echo 'main.o: ' >> Makefile
echo -n -e '\t' >> Makefile
echo "g++ --std=c++14 -c main.cpp" >> Makefile

make -j10

cd -

#	g++ --std=c++14 -o "$wd"/analyzer_bin *cpp
