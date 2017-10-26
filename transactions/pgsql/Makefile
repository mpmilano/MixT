TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
CPPFLAGS= -I$(PWD) -I$(PWD)/../ --std=c++1z -Wall -DNUM_CAUSAL_GROUPS="4" -DCAUSAL_GROUP=$(causalGroup) -DSTRONG_REMOTE_IP=\"$(STRONG_REMOTE_IP)\" -DCAUSAL_REMOTE_IP_1=\"$(CAUSAL_REMOTE_IP_1)\" -DCAUSAL_REMOTE_IP_2=\"$(CAUSAL_REMOTE_IP_2)\" -DMY_IP=\"$(MY_IP)\" -DMAX_THREADS=$(MAX_THREADS) -ferror-limit=1 -Wall -Werror -Wextra -DNUM_CAUSAL_MASTERS="2" -O2  -DNDEBUG 
LDFLAGS=  --std=c++1z -lpqxx -lm -pthread 
client_object_files=utils.o Basics.o Ostreams.o SerializationSupport.o SQLStore.o GSQLObject.o SQLConnection.o SQLTransaction.o SQL_internal_utils.o SQLLevels.o TransactionContext.o ServerSocket.o Socket.o simple_rpc.o local_connection.o Tombstone.o ClockManager.o Tracker.o Ends.o

export IP_quad
export TRANS_DIR
export CPPFLAGS
export LDFLAGS

all:
	echo "don't use this target"

strong: $(object_files)
	clang++ LocalSQLReceiver.cpp $(object_files) -o strong_receiver -DRCVR_LEVEL=Level::strong $(CPPFLAGS) $(LDFLAGS)

causal: $(object_files)
	clang++ LocalSQLReceiver.cpp $(object_files) -o causal_receiver -DRCVR_LEVEL=Level::causal $(CPPFLAGS) $(LDFLAGS)

strong_relay: $(client_object_files)
	clang++ relay.cpp $(client_object_files) -o strong_relay -DSTORE_LEVEL=strong $(CPPFLAGS) $(LDFLAGS)

causal_relay: $(client_object_files)
	clang++ relay.cpp $(client_object_files) -o causal_relay -DSTORE_LEVEL=causal $(CPPFLAGS) $(LDFLAGS)

strong_relay_tracked: $(client_object_files)
	clang++ -DTRACK relay.cpp $(client_object_files) -o strong_relay_tracked -DSTORE_LEVEL=strong $(CPPFLAGS) $(LDFLAGS)

causal_relay_tracked: $(client_object_files)
	clang++ -DTRACK relay.cpp $(client_object_files) -o causal_relay_tracked -DSTORE_LEVEL=causal $(CPPFLAGS) $(LDFLAGS)

test: $(object_files)
	clang++ test_local_transactions.cpp $(object_files) -o test $(CPPFLAGS) $(LDFLAGS)

test_serialization: $(client_object_files)
	clang++ test_relay_serialization.cpp $(client_object_files) -o test_serialization $(CPPFLAGS) $(LDFLAGS)
Tombstone.o:
	clang++ -c ../*/Tombstone.cpp $(CPPFLAGS)
oid_translator.o:
	clang++ -c -O3 oid_translator.cpp $(CPPFLAGS)
pgexceptions.o:
	clang++ -c -O3 pgexceptions.cpp $(CPPFLAGS)
pgdeferred.o:
	clang++ -c -O3 pgdeferred.cpp $(CPPFLAGS)
pgresult.o:
	clang++ -c -O3 pgresult.cpp $(CPPFLAGS)
pgtransaction.o:
	clang++ -c -O3 pgtransaction.cpp $(CPPFLAGS)
LocalSQLConnection.o:
	clang++ -c -O3 LocalSQLConnection.cpp $(CPPFLAGS)
SQLConstants.o:
	clang++ -c -O3 SQLConstants.cpp $(CPPFLAGS)
batched_connection_server.o:
	clang++ -c -O3 ../*/batched_connection_server.cpp $(CPPFLAGS)

Ostreams.o:
	clang++ -c ../Ostreams.cpp $(CPPFLAGS)

simple_rpc.o:
	clang++ -c ../*/simple_rpc.cpp $(CPPFLAGS)

eventfd.o:
	clang++ -c ../*/eventfd.cpp $(CPPFLAGS)

epoll.o:
	clang++ -c ../*/epoll.cpp $(CPPFLAGS)

dual_connection.o:
	clang++ -c -O3  ../*/dual_connection.cpp $(CPPFLAGS)
dual_connection_superstate.o:
	clang++ -c -O3  ../*/dual_connection_superstate.cpp $(CPPFLAGS)

batched_connection_client.o:
	clang++ -c -O3 ../*/batched_connection_client.cpp $(CPPFLAGS)

batched_connection_common.o:
	clang++ -c -O3 ../*/batched_connection_common.cpp $(CPPFLAGS)
ClockManager.o:
	clang++ -c ../*/ClockManager.cpp $(CPPFLAGS)
ServerSocket.o:
	clang++ -c ../*/ServerSocket.cpp $(CPPFLAGS)
SQLConnection.o:
	clang++ -c ../*/SQLConnection.cpp $(CPPFLAGS)

Socket.o:
	clang++ -c ../*/Socket.cpp $(CPPFLAGS)

SerializationSupport.o:
	clang++ -c ../*/SerializationSupport.cpp $(CPPFLAGS)
local_connection.o:
	clang++ -c ../*/local_connection.cpp $(CPPFLAGS)
Basics.o:
	clang++ -c ../Basics.cpp $(CPPFLAGS)
Tracker.o:
	clang++ -c ../*/Tracker.cpp $(CPPFLAGS) #&& ln -s DisabledTracker.o Tracker.o
TransactionContext.o:
	clang++ -c ../*/TransactionContext.cpp $(CPPFLAGS)
SQLLevels.o:
	clang++ -c SQLLevels.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQL_internal_utils.o:
	clang++ -c SQL_internal_utils.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQLTransaction.o:
	clang++ -c SQLTransaction.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
GSQLObject.o:
	clang++ -c GSQLObject.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQLStore.o:
	clang++ -c SQLStore.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
abiutils.o:
	clang++ -c ../*/abiutils.cpp $(CPPFLAGS)
Ends.o:
	clang++ -c ../*/Ends.cpp $(CPPFLAGS)
utils.o:
	clang++ -c ../*/utils.cpp $(CPPFLAGS)
clean:
	rm *.o; rm strong_receiver; rm causal_receiver;
