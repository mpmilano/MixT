if ssh $* ./low_ram.sh; then
		echo "low ram";
		killall strong_relay;
		killall causal_relay;
		killall strong_relay_tracked;
		killall causal_relay_tracked;
		killall simple_txn_test;
		killall simple_txn_test_tracked;
		killall mailing_list_test;
else
		echo fine;
fi
