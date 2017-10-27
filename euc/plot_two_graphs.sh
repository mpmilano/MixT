#!/bin/bash
which_prefix_1="$1" #~research/results/
which_prefix_2="$2" #~research/results/
which_directories="$3" #MyriaStore-.05-.05
for foo in "$which_directories"; do cat $which_prefix_1/$foo/throughput*latency* | grep -v Myria | sed -e s/_Hz//g -e s/us//g -e s/ms//g | tr ',' ' ' > /tmp/plot1; cat $which_prefix_2/$foo/throughput*latency* |  grep -v Myria | sed -e s/_Hz//g -e s/us//g -e s/ms//g | tr ',' ' ' > /tmp/plot2; gnuplot -p -e "set title '$foo'" -e "set term png" -e "set output '$foo.png'" -e "plot '/tmp/plot1' using 3:1, '/tmp/plot2' using 3:1"; done
