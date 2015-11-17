#!/bin/bash
i=0
for num_per in 1 #2 3 4 5 6 7 8 9 10
do
	for configuration in USE_STRONG #NO_USE_STRONG
	for foo in $instance_list
	do
		i=$[i%4 + 1]
		/bin/bash test-things-loop-body.sh $i $foo $num_per $configuration &
	done
	wait
	echo "all done"
	for foo in $instance_list
	do
		i=$[i%4 + 1]
		/bin/bash test-things-loop-body2.sh $i $foo $num_per $configuration &
	done
	wait
	echo copy done
done
