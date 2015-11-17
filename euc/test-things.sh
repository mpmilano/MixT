#!/bin/bash
i=0
for foo in $instance_list
do
	i=$[i%4 + 1]
	/bin/bash test-things-loop-body.sh $i $foo &
done
