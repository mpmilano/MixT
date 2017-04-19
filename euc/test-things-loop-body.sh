i=$1
foo="$2"
percent_read="$3"
percent_causal="$4"
strong_target=$5
causal_target_1=$6
causal_target_2=$7
num_clients=$8
client_rate=$9
shift
client_increase_rate=$9
shift
test_stop_time=$9
shift
max_threads=$9
shift
first_iter=$9

scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem vm-actions.sh ubuntu@"$foo":vm-actions.sh
scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem as-gentoo.sh ubuntu@"$foo":as-gentoo.sh
ssh  -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@"$foo" sudo /bin/bash vm-actions.sh $i "$foo" $ndebug $strong_target $causal_target_1 $causal_target_2 $num_clients $client_rate $client_increase_rate $test_stop_time $percent_causal $percent_read $max_threads $first_iter
