#!/bin/bash
alias make="/usr/bin/make -j 8"
export PGHOST='/run/postgresql';
#export PGPORT='5432';
export PGDATABASE='DataStore';
export PGUSER='research';
export PGPASSWORD='researchVM'
export extra_macro_defs='-DNORMAL -DWRITE_PERCENT=.05 -DSTRONG_PERCENT=.05'
export causalGroup=3
export MY_IP=128.84.217.139
export STRONG_REMOTE_IP=128.253.3.197
export CAUSAL_REMOTE_IP_1=128.84.217.139
export CAUSAL_REMOTE_IP_2=$CAUSAL_REMOTE_IP_1
export MAX_THREADS=3000
export increase_by=10
export increase_delay=5s
export test_stop_time=1min
export num_clients=30
export client_rate=10_Hz
#export instance_list=
function ssh-euc() {
	host=$1;
	shift;
	ssh -o "UserKnownHostsFile /dev/null"  -o strictHostKeyChecking=no -i ~/research/euc/MyriaInstances.pem ubuntu@$host $*;
}
export -f ssh-euc
