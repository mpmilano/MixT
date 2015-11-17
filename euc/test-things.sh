#!/bin/bash
i=0
for foo in $instance_list
do
	i=$[i%4 + 1]
	scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem vm-actions.sh ubuntu@"$foo":vm-actions.sh
	scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem as-gentoo.sh ubuntu@"$foo":as-gentoo.sh
	if [[ $[1 + (i - 1)/2 ] = 1 ]]
	then
		#ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -R 5432:128.84.217.87:5432 -i MyriaInstances.pem ubuntu@"$foo" sudo /bin/bash vm-actions.sh $i &
		ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -R 5432:localhost:5432 -i MyriaInstances.pem ubuntu@"$foo" sudo /bin/bash vm-actions.sh $i &			
	else 
		ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -R 5432:localhost:5432 -i MyriaInstances.pem ubuntu@"$foo" sudo /bin/bash vm-actions.sh $i &		
	fi
done
