#!/bin/bash

echo "$STRONG_REMOTE_IP, 8876, $CAUSAL_REMOTE_IP_1, 8877, $client_rate, $num_clients, $client_increase_rate, $test_stop_time, 0.01, $percent_causal, $percent_write, /tmp/MyriaStore-results, 60s" | ./simple_txn_test 
