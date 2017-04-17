#!/bin/bash
alias make="/usr/bin/make -j 8"
export PGHOST='/run/postgresql';
#export PGPORT='5432';
export PGDATABASE='DataStore';
export PGUSER='research';
export PGPASSWORD='researchVM'
export causalGroup=3
export MY_IP=128.84.217.139
export STRONG_REMOTE_IP=128.253.3.197
export strong_target=$STRONG_REMOTE_IP
export CAUSAL_REMOTE_IP_1=128.84.217.139
export causal_target_1=$CAUSAL_REMOTE_IP_1
export CAUSAL_REMOTE_IP_2=$CAUSAL_REMOTE_IP_1
export causal_target_2=$CAUSAL_REMOTE_IP_2
export MAX_THREADS=3000
export max_threads=$MAX_THREADS
export client_increase_rate=2_Hz
export test_stop_time=6min
export num_clients=510
export client_rate=10_Hz
export percent_causal=.95
export causal_percentages=$percent_causal
export percent_read=.95
export read_percentages=$percent_read
#export instance_list=
function ssh-euc() {
	host=$1;
	shift;
	ssh -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -i ~/research/euc/MyriaInstances.pem ubuntu@$host $*;
}
export -f ssh-euc
