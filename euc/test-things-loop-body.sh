i=$1
foo="$2"
configuration=$3
ndebug=$4
shift
write_percent=$4
strong_percent=$5
strong_target=$6
causal_target_1=$7
causal_target_2=$8
num_clients=$9
shift
client_rate=$9
shift
increase_by=$9
shift
increase_delay=$9
shift
test_stop_time=$9
shift
max_threads=$9
shift
first_iter=$9

echo $increase_by

scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem vm-actions.sh ubuntu@"$foo":vm-actions.sh
scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem as-gentoo.sh ubuntu@"$foo":as-gentoo.sh
ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@"$foo" -t sudo /bin/bash vm-actions.sh $i $configuration $write_percent $strong_percent "$foo" $ndebug $strong_target $causal_target_1 $causal_target_2 $num_clients $client_rate $increase_by $increase_delay $test_stop_time $max_threads $first_iter
