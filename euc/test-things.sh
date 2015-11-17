#!/bin/bash
i=0
for foo in $instance_list
do
	i=$[i%4 + 1]
	/bin/bash test-things-loop-body.sh $i $foo &
done
wait
echo "all done"
for foo in $instance_list
do
	i=$[i%4 + 1]
	/bin/bash test-things-loop-body2.sh $i $foo &
done
wait
echo copy done
