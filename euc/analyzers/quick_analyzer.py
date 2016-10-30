#!/usr/bin/env python3

#idea; this script expects to take the file on stdin and
#some expression to be run against it as a string in $*.
#makes the various fields available as bash variables

#struct myria_log { const int submit_time; const int run_time; const int cc_num_tries; const int done_time; const bool is_write; const bool is_read; const bool is_strong; const bool is_causal; const bool is_serialization_error; const std::string remote_failure_string; const bool remote_failure; const int num_causal_tries; const bool transaction_action; const int tracker_strong_afterread_tombstone_exists; const int tracker_strong_afterread_nonce_unavailable; const int tracker_causal_afterread_candidate;  }; 

#[]() -> struct myria_log {struct myria_log ret {1743, 1743, 0, 1760, false, 1, false, 1, false, "", false, 1, 1, 0, 0, 0}; return ret; }()

import sys
import functools
from collections import namedtuple


def concat_all(l):
    r=""
    for s in l:
        r += s
    return r

myria_log = namedtuple("myria_log",['submit_time', 'run_time', 'cc_num_tries', 'done_time', 'is_write', 'is_read', 'is_strong', 'is_causal', 'is_serialization_error', 'remote_failure_string', 'remote_failure', 'num_causal_tries', 'transaction_action', 'tracker_strong_afterread_tombstone_exists', 'tracker_strong_afterread_nonce_unavailable', 'tracker_causal_afterread_candidate'])

def compose(*functions):
    return functools.reduce(lambda f, g: lambda x: f(g(x)), functions, lambda x: x)

def stream_operator(preprocess_fun):
    to_return=[]
    for line in sys.stdin.readlines():
        if '[]() -> struct myria_log {struct myria_log ret' in line:
            entries = line.split(sep='{')[-1].split(sep='}')[0].split(sep=', ')
            to_return.append(
                preprocess_fun(
                    myria_log(
                        submit_time=int(entries[0]),
                        run_time=int(entries[1]),
                        cc_num_tries=int(entries[2]),
                        done_time=int(entries[3]),
                        is_write=bool(entries[4]),
                        is_read=bool(entries[5]),
                        is_strong=bool(entries[6]),
                        is_causal=bool(entries[7]),
                        is_serialization_error=bool(entries[8]),
                        remote_failure_string=entries[9],
                        remote_failure=bool(entries[10]),
                        num_causal_tries=int(entries[11]),
                        transaction_action=bool(entries[12]),
                        tracker_strong_afterread_tombstone_exists=int(entries[13]),
                        tracker_strong_afterread_nonce_unavailable=int(entries[14]),
                        tracker_causal_afterread_candidate=int(entries[15]))))
    return to_return

    
'''
function set_vars {
	submit_time=$1
	run_time=$2
	cc_num_tries=$3
	done_time=$4
	is_write=$5
	is_read=$6
	is_strong=$7
	is_causal=$8
	is_serialization_error=$9
	remote_failure_string="$10"
	remote_failure=$11
	num_causal_tries=$12
	transaction_action="$13"
	tracker_strong_afterread_tombstone_exists=$14
	tracker_strong_afterread_nonce_unavailable=$15
	tracker_causal_afterread_candidate=$16
}

echo $sed_command
while read line; do
	if [[ `echo "$line" | grep '\[\]\(\)' | grep myria_log` ]]; then
		shortened_line="`echo "$line" | rev | cut -d '{' -f1 | rev | cut -d'}' -f1 | tr ',' ' '  | sed s/'  '/' '/g`"
		set_vars $shortened_line
		eval $*
	fi
done < /dev/stdin


'''
