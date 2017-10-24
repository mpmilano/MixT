#!/bin.bash
#we expect three arguments:  arg1 : window increment; arg2: window duration; arg3: test run result file
./main $* | grep "us$" | tr , ' ' | awk '{print $1,$2}' | sed s/_Hz// | sed s/ms// | gnuplot -p -e "set title 'TL'" -e "plot '<cat' using 1:2"
