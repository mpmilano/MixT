TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
CPPFLAGS= -I$(PWD) -Wno-unused-local-typedefs --std=c++1z -DNUM_CAUSAL_GROUPS="4" -DMY_IP=\"$(MY_IP)\" -DMAX_THREADS=$(MAX_THREADS) -DIP_QUAD=$(IP_quad) -DSTRONG_REMOTE_IP=\"$(STRONG_REMOTE_IP)\" -DCAUSAL_GROUP=$(causalGroup) -DCAUSAL_REMOTE_IP_1=\"$(CAUSAL_REMOTE_IP_1)\" -DCAUSAL_REMOTE_IP_2=\"$(CAUSAL_REMOTE_IP_2)\" -Wall -Werror -Wextra  -O2 -fPIC 
LDFLAGS=  --std=c++1z -lm -pthread -lpqxx -ftemplate-depth=10000
object_files=Ends.o utils.o test_utils.o Basics.o Tracker.o Ostreams.o SerializationSupport.o SQLStore.o GSQLObject.o SQLConnection.o SQLTransaction.o SQL_internal_utils.o SQLLevels.o TransactionContext.o ServerSocket.o Socket.o eventfd.o epoll.o simple_rpc.o local_connection.o Tombstone.o ClockManager.o configuration_params.o run_result.o
mailing_list_object_files=mailing_list_post_new_message.o mailing_list_add_new_user.o mailing_list_download_inbox.o mailing_list_create_group.o mailing_list_create_user.o mailing_list_example.o
chosen_compiler=clang++-5.0 -ferror-limit=1 -fconstexpr-steps=40048576
all: $(object_files) 
	@clang++ $(object_files)  -o test $(LDFLAGS)

sample_arrival_intervals: utils.o test_utils.o
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) utils.o test_utils.o sample_arrival_intervals.cpp -o sample
test_parse_utilities: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) $(object_files) mtl/test_parse_utilities.cpp -o test_parse_utils
test_testing_store: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) $(object_files) test_testing_store.cpp -o test_store
test_parsing: utils.o
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) utils.o test_parsing.cpp -o /tmp/test_parsing
test_binop: utils.o
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) utils.o test_binop.cpp -o /tmp/test_binop
test_deref: utils.o
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) utils.o deref_test.cpp -o /tmp/test
test_mtl: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) $(object_files) test_complex_transaction.cpp -o /tmp/test
test_method: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) $(object_files) test_mixt_method.cpp -o /tmp/test
test_conversion: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS) $(object_files) test_conversion.cpp -o /tmp/test
test_operation: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) test_operation.cpp -o /tmp/test
logging_example: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) logging_example.cpp -o /tmp/test
voting_example: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) voting_example.cpp -o /tmp/test
mailing_list_test: $(object_files) $(mailing_list_object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) $(mailing_list_object_files) mailing_list_test.cpp -o mailing_list_test

test_mailing_list: $(object_files) $(mailing_list_object_files) test_mailing_list.cpp
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) $(mailing_list_object_files) test_mailing_list.cpp -o test_mailing_list
test_handle_serialization: $(object_files) test_handle_serialization.cpp
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) test_handle_serialization.cpp -o test_handle_serialization
test_rawsql: $(object_files) test_rawsql.cpp raw_sqlstore/RawSQLStore.hpp
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) test_rawsql.cpp -o test_rawsql
mailing_list_example.o: $(object_files) mailing_list_example.cpp
	$(chosen_compiler) -c mailing_list_example.cpp $(CPPFLAGS) -ftemplate-depth=10000
mailing_list_post_new_message.o: $(object_files) mailing_list_post_new_message.cpp mtl/run_phase.hpp
	$(chosen_compiler)  -DUSE_PRECOMPILED -c mailing_list_post_new_message.cpp $(CPPFLAGS) -ftemplate-depth=10000
mailing_list_download_inbox.o: $(object_files) mailing_list_download_inbox.cpp mtl/run_phase.hpp
	$(chosen_compiler)  -DUSE_PRECOMPILED -c mailing_list_download_inbox.cpp $(CPPFLAGS) -ftemplate-depth=10000	
mailing_list_add_new_user.o: $(object_files) mailing_list_add_new_user.cpp mtl/run_phase.hpp
	$(chosen_compiler)  -DUSE_PRECOMPILED -c mailing_list_add_new_user.cpp $(CPPFLAGS) -ftemplate-depth=10000
mailing_list_create_user.o: $(object_files) mailing_list_create_user.cpp mtl/run_phase.hpp
	$(chosen_compiler)  -DUSE_PRECOMPILED -c mailing_list_create_user.cpp $(CPPFLAGS) -ftemplate-depth=10000
mailing_list_create_group.o: $(object_files) mailing_list_create_group.cpp mtl/run_phase.hpp
	$(chosen_compiler)  -DUSE_PRECOMPILED -c mailing_list_create_group.cpp $(CPPFLAGS) -ftemplate-depth=10000
test_endorse: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) test_endorse.cpp -o /tmp/test
debug_scratch: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) debug_scratch.cpp -o /tmp/test
test_builtins: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) test_builtins.cpp -o /tmp/test
regression_tests: $(object_files)
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  $(object_files) regression_tests.cpp -o /tmp/test
test_isvalid: utils.o
	$(chosen_compiler) $(CPPFLAGS) $(LDFLAGS)  test_isvalid.cpp utils.o -o /tmp/test

test_causal: $(object_files) 
	$(chosen_compiler) test_causal.cpp DisabledTracker.o SQLStore.o Ends.o utils.o Basics.o Ostreams.o SerializationSupport.o GSQLObject.o SQLConnection.o SQLTransaction.o SQL_internal_utils.o ServerSocket.o Socket.o batched_connection_client.o batched_connection_common.o  -o test_causal $(CPPFLAGS) $(LDFLAGS)

stress1: $(object_files) strxess_test1.o
	$(chosen_compiler) $(object_files) stress_test.o -o stress1 $(LDFLAGS)

stress2: $(object_files) stress_test2.o
	$(chosen_compiler) $(object_files) stress_test.o -o stress2 $(LDFLAGS)

stress3:  $(object_files) stress_test3.o
	$(chosen_compiler) $(object_files) stress_test.o -o stress3 $(LDFLAGS)

simple_txn_test: $(object_files) simple_txn_test.cpp
	$(chosen_compiler) simple_txn_test.cpp $(object_files) $(CPPFLAGS) $(LDFLAGS) -o simple_txn_test

simple_txn_test_tracked: $(object_files) simple_txn_test.cpp
	$(chosen_compiler) -DTRACK simple_txn_test.cpp $(object_files) $(CPPFLAGS) $(LDFLAGS) -o simple_txn_test_tracked

config_params_parsing_test: utils.o
	$(chosen_compiler) config_params_parsing_test.cpp utils.o -o config_params_parsing_test $(CPPFLAGS) $(LDFLAGS)

vm: simple_txn_test simple_txn_test.cpp
	cp -rfp vm.sh vm
groups: $(object_files) groups_test.o
	$(chosen_compiler) $(object_files) groups_test.o -o groups $(LDFLAGS)
insert_tracking: $(object_files) 
	$(chosen_compiler) $(object_files) test_insert_tracking.cpp -o test_insert_tracking $(LDFLAGS) $(CPPFLAGS)

cache_test: $(object_files) cache_interacter.o
	$(chosen_compiler) $(object_files) cache_interacter.o -o cache_test $(LDFLAGS)

cache_interacter.o:
	$(chosen_compiler) -c cache_interacter.cpp $(CPPFLAGS)

chat: $(object_files) chat_example.o
	$(chosen_compiler) $(object_files) chat_example.o -o chat $(LDFLAGS)

sqlstore: sqlstore_clean all
	echo "done"

tracker: tracker_clean all
	echo "done"

run_result.o:
	$(chosen_compiler) -c run_result.cpp $(CPPFLAGS)
configuration_params.o:
	$(chosen_compiler) -c configuration_params.cpp $(CPPFLAGS)
stress_test1.o:
	$(chosen_compiler) -c stress_test.cpp -DTEST_MODE=1 $(CPPFLAGS)
stress_test2.o:
	$(chosen_compiler) -c stress_test.cpp -DTEST_MODE=2 $(CPPFLAGS)
stress_test3.o:
	$(chosen_compiler) -c stress_test.cpp -DTEST_MODE=3 $(CPPFLAGS)
vm_main.o:
	$(chosen_compiler) -c vm_main.cpp -DCLIENT_RATE=$(client_rate) $(CPPFLAGS)
groups_test.o:
	$(chosen_compiler) -c groups_test.cpp  $(CPPFLAGS)
sqlstore_clean:
	@touch SQLStore.o; rm SQLStore.o
tracker_clean:
	@rm Tracker.o;  true
chat_example.o:
	$(chosen_compiler) -c chat_example.cpp $(CPPFLAGS)
Basics.o:
	$(chosen_compiler) -c Basics.cpp $(CPPFLAGS)
utils.o:
	$(chosen_compiler) -c */utils.cpp $(CPPFLAGS)
epoll.o:
	$(chosen_compiler) -c */epoll.cpp $(CPPFLAGS)
test_utils.o:
	$(chosen_compiler) -c test_utils.cpp $(CPPFLAGS)
Profiler.o:
	$(chosen_compiler) -c */Profiler.cpp $(CPPFLAGS)
GC_pointer.o:
	$(chosen_compiler) -c */GC_pointer.cpp $(CPPFLAGS)
eventfd.o:
	$(chosen_compiler) -c */eventfd.cpp $(CPPFLAGS)
Ends.o:
	$(chosen_compiler) -c */Ends.cpp $(CPPFLAGS)
Tracker.o:
	$(chosen_compiler) -c */Tracker.cpp $(CPPFLAGS) #&& ln -s DisabledTracker.o Tracker.o
Tombstone.o:
	$(chosen_compiler) -c */Tombstone.cpp $(CPPFLAGS)
ClockManager.o:
	clang++ -c */ClockManager.cpp $(CPPFLAGS)
TransactionContext.o:
	$(chosen_compiler) -c */TransactionContext.cpp $(CPPFLAGS)
Ostreams.o:
	$(chosen_compiler) -c Ostreams.cpp $(CPPFLAGS)
SerializationSupport.o:
	$(chosen_compiler) -c */SerializationSupport.cpp $(CPPFLAGS)
ServerSocket.o:
	clang++ -c */ServerSocket.cpp $(CPPFLAGS)
Socket.o:
	$(chosen_compiler) -c */Socket.cpp $(CPPFLAGS)
SQLStore.o:
	$(chosen_compiler) -c */SQLStore.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQLConstants.o:
	$(chosen_compiler) -c */SQLConstants.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
GSQLObject.o:
	$(chosen_compiler) -c */GSQLObject.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQLConnection.o:
	$(chosen_compiler) -c */SQLConnection.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQLTransaction.o:
	$(chosen_compiler) -c */SQLTransaction.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQL_internal_utils.o:
	$(chosen_compiler) -c */SQL_internal_utils.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQLLevels.o:
	$(chosen_compiler) -c */SQLLevels.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
abiutils.o:
	$(chosen_compiler) -c */abiutils.cpp $(CPPFLAGS)
batched_connection_server.o:
	$(chosen_compiler) -c -O3 */batched_connection_server.cpp $(CPPFLAGS)

batched_connection_client.o:
	$(chosen_compiler) -c -O3 */batched_connection_client.cpp $(CPPFLAGS)

batched_connection_common.o:
	$(chosen_compiler) -c -O3 */batched_connection_common.cpp $(CPPFLAGS)
dual_connection.o:
	$(chosen_compiler) -c -O3  */dual_connection.cpp $(CPPFLAGS)
simple_rpc.o:
	$(chosen_compiler) -c */simple_rpc.cpp $(CPPFLAGS)
local_connection.o:
	$(chosen_compiler) -c */local_connection.cpp $(CPPFLAGS)
proxy_connection.o:
	$(chosen_compiler) -c */proxy_connection.cpp $(CPPFLAGS)


run: test
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand' ./test

debug: test
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand'  gdb ./test



clean:
	rm *.o; rm vm
