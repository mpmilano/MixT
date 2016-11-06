#!/bin/bash
alias make="/usr/bin/make -j 8"
export PGHOST='/run/postgresql';
#export PGPORT='5432';
export PGDATABASE='DataStore';
export PGUSER='research';
export PGPASSWORD='researchVM'
export extra_macro_defs='-DNORMAL -DWRITE_PERCENT=.3 -DSTRONG_PERCENT=.2'
export causalGroup=3
export MY_IP=128.84.217.139
export STRONG_REMOTE_IP=128.253.3.197
export CAUSAL_REMOTE_IP_1=128.84.217.139
export CAUSAL_REMOTE_IP_2=$CAUSAL_REMOTE_IP_1
export MAX_THREADS=1000
export increase_by=20_Hz
export increase_delay=3s
export test_stop_time=3min
export num_clients=20
export client_rate=5_Hz
