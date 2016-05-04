#!/bin/bash

if [[ -d "$1" ]]
then results_dir=`readlink -f $1`
else echo "Error: results dir (first argument) must be directory. Got $1"
	 exit 1
fi

if [[ -d "$2" ]]
then mutils=`readlink -f $2`
else echo "Error: mutils dir (second argument) must be directory. Got $2"
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
echo "#include <Hertz.hpp>" >> "$scratchdir"/header.hpp
cat "$results_dir"/*/*/* | grep 'struct myria_log' | sed s/'const'//g | head -1 >> "$scratchdir"/header.hpp
cat "$results_dir"/*/*/* | grep 'struct myria_globals' | head -1 >> "$scratchdir"/header.hpp

for mode____ in "$results_dir"/*/;
do
	mode=`echo $mode____ | rev | cut -d'/' -f2 | rev`
	echo $mode
	for ____ipaddr in "$results_dir"/"$mode"/*/
	do
		ipaddr=`echo $____ipaddr | rev | cut -d'/' -f2 | rev`
		host=`echo "$ipaddr" | cut -d'.' -f4`
		fname=$scratchdir/"output_""$mode""_""$host"
		echo $fname
		echo "#include \"header.hpp\"" > "$fname".hpp
		echo "#include \"$fname"".hpp\"" > "$fname".cpp
		
		echo "std::vector<struct myria_log> runs_""$mode""_$host""();" >> "$fname".hpp
		echo "std::vector<struct myria_log> runs_""$mode""_$host""() { return std::vector<struct myria_log> {{" >> "$fname".cpp
		cat "$results_dir"/"$mode"/"$ipaddr"/* | grep '\[\]' | grep 'myria_log' | sed s/'const'// | sed s/'\[\]() -> struct myria_log {struct myria_log ret '/'myria_log'/ | sed s/'; return ret; }()'// | sed -e "$ ! s/\$/,/g" >>"$fname".cpp
		echo '}};}' >> "$fname".cpp
		
		echo "std::vector<struct myria_globals> globals_""$mode""_$host""();" >> "$fname".hpp
		echo "std::vector<struct myria_globals> globals_""$mode""_$host""() { using namespace mutils; return std::vector<struct myria_globals> {{" >> "$fname".cpp
		cat "$results_dir"/"$mode"/"$ipaddr"/* | grep '\[\]' | grep 'myria_globals' | sed -e "$ ! s/\$/,/g" >>"$fname".cpp
		echo '}};}' >> "$fname".cpp
	done
done

cat "$scratchdir"/output_*hpp > "$scratchdir"/results_header.hpp
wd=`pwd`
cd "$scratchdir"

pwd
#build the makefile
names=""
for mode____ in `ls -d $results_dir/*/`;
do
	for ____ipaddr in "$results_dir"/"$mode"/*/
	do
		ipaddr=`echo $____ipaddr | rev | cut -d'/' -f2 | rev`
		host=`echo "$ipaddr" | cut -d'.' -f4`
		mode=`echo $mode____ | rev | cut -d'/' -f2 | rev`
		names="$names output_$mode"_"$host"
	done
done

objects=""
for foo in $names;
do objects="$objects $foo".o
done

echo "all: $objects" > Makefile
echo -n -e '\t' >> Makefile
echo "echo objects built" >> Makefile

for name in $names
do
	echo "$name"".o:" >> Makefile
	echo -n -e '\t' >> Makefile
	echo "clang++ -I""$mutils"" --std=c++14 -c $name".cpp >> Makefile
done

echo "#pragma once" > unified_results_header.hpp
for foo in *hpp
do
	echo "#include \"$foo\"" >> unified_results_header.hpp
	echo >> unified_results_header.hpp
done

for mode____ in "$results_dir"/*/;
do
	mode=`echo $mode____ | rev | cut -d'/' -f2 | rev`
	echo "const std::vector<myria_log>& all_logs_""$mode""() { static std::vector<myria_log> ret; if (ret.size() > 5) return ret; " >> unified_results_header.hpp
	for ____ipaddr in "$results_dir"/"$mode"/*/
	do
		ipaddr=`echo $____ipaddr | rev | cut -d'/' -f2 | rev`
		host=`echo "$ipaddr" | cut -d'.' -f4`
		
		echo "{ auto tmp = runs_""$mode""_$host""(); ret.insert(ret.end(),tmp.begin(),tmp.end()); }" >> unified_results_header.hpp
	done
	
	echo "return ret; }" >> unified_results_header.hpp
done

make

cd -

cp "$scratchdir"/output*.o $results_dir/
cp "$scratchdir"/*.hpp $results_dir/

#	
