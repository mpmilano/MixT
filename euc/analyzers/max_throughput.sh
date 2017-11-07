#!/bin/bash

./main $1 $2 $3 | grep "us$" | tr , ' ' | awk '{print $2,$1}' | grep -v [2-9][0-9][0-9][0-9]ms | grep -v [0-9][0-9][0-9][0-9][0-9]*ms | cut -d' ' -f2 | cut -d'_' -f1 | sort -n | tail -1 > $4 
