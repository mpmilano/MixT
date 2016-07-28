#!/bin/bash
alias make="/usr/bin/make -j 8"
export PGHOST='localhost';
export PGPORT='5432';
export PGDATABASE='DataStore';
export PGUSER='research';
export PGPASSWORD='researchVM'
export extra_macro_defs='-DNORMAL -DWRITE_PERCENT=.3 -DSTRONG_PERCENT=.2'
export causalGroup=3
export MY_IP=128.84.217.139
export STRONG_REMOTE_IP=128.84.217.139
export MAX_THREADS=10000

