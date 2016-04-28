i=$1
foo="$2"
configuration=$3
strong_target=$4
causal_target_1=$5
causal_target_2=$6
first_iter=$7

scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem vm-actions.sh ubuntu@"$foo":vm-actions.sh
scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem as-gentoo.sh ubuntu@"$foo":as-gentoo.sh
if [[ $[1 + (i - 1)/2 ] = 1 ]]
then
	ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -R 5432:"$causal_target_1":5432 -i MyriaInstances.pem ubuntu@"$foo" sudo /bin/bash vm-actions.sh $i $configuration "$foo" $strong_target $first_iter
else 
	ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -R 5432:"$causal_target_2":5432 -i MyriaInstances.pem ubuntu@"$foo" sudo /bin/bash vm-actions.sh $i $configuration "$foo" $strong_target $first_iter
fi
