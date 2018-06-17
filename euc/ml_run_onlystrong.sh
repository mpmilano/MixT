#!/bin/bash

function reset {
		ssh research@research.xelserv.com killall causal_relay; ssh research@research.xelserv.com killall strong_relay; sleep 15;
}

function mailing_list {
		./mailing_list_test 128.253.3.197 8876 128.253.3.197 8877 10_Hz 100 5_Hz 9min 0.01 $1 $2 /tmp/ml-$1-$2-$3 5s 10 8
}

reset
mailing_list 0 0 1
reset
mailing_list 0 0 2
reset
mailing_list 0 0 3
reset
mailing_list 0 0 4
reset

mailing_list 1 0 1
reset
mailing_list 1 0 2
reset
mailing_list 1 0 3
reset
mailing_list 1 0 4
reset

mailing_list 0 1 1
reset
mailing_list 0 1 2
reset
mailing_list 0 1 3
reset
mailing_list 0 1 4
reset

mailing_list 1 1 1
reset
mailing_list 1 1 2
reset
mailing_list 1 1 3
reset
mailing_list 1 1 4
reset
