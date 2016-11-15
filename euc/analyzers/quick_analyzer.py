#!/usr/bin/env python3

#idea; this script expects to take the file on stdin and
#some expression to be run against it as a string in $*.
#makes the various fields available as bash variables

import sys
import functools
from collections import namedtuple


def concat_all(l):
    r=""
    for s in l:
        r += s
    return r

myria_log = namedtuple("myria_log",['submit_time', 'run_time', 'cc_num_tries', 'done_time', 'is_write', 'is_read', 'is_strong', 'is_causal', 'remote_failure_string', 'num_causal_tries', 'transaction_action', 'tracker_strong_afterread_tombstone_exists', 'tracker_strong_afterread_nonce_unavailable', 'tracker_causal_afterread_candidate'])

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
                        remote_failure_string=entries[8],
                        num_causal_tries=int(entries[9]),
                        transaction_action=bool(entries[10]),
                        tracker_strong_afterread_tombstone_exists=int(entries[11]),
                        tracker_strong_afterread_nonce_unavailable=int(entries[12]),
                        tracker_causal_afterread_candidate=int(entries[13]))))
    return to_return
