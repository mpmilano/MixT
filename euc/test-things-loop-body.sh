i=$1
foo="$2"
configuration=$3
write_percent=$4
strong_percent=$5
strong_target=$6
causal_target_1=$7
causal_target_2=$8
first_iter=$9

scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem vm-actions.sh ubuntu@"$foo":vm-actions.sh
scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem as-gentoo.sh ubuntu@"$foo":as-gentoo.sh
if [[ $[1 + (i - 1)/2 ] = 1 ]]
then
	ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@"$foo" sudo /bin/bash vm-actions.sh $i $configuration $write_percent $strong_percent "$foo" $strong_target $causal_target_1 $causal_target_2 $first_iter
else 
	ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@"$foo" sudo /bin/bash vm-actions.sh $i $configuration $write_percent $strong_percent "$foo" $strong_target $causal_target_1 $causal_target_2 $first_iter
fi
