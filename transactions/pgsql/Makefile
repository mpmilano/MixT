TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
storeList='pgsql::SQLStore<Level::strong>, pgsql::SQLStore<Level::causal>'
CPPFLAGS= -I$(PWD) -I$(PWD)/../ -I$(PWD)/../mtl -I$(PWD)/..//myria-utils -I$(PWD)/../mutils-networking -I$(PWD)/../mutils -I$(PWD)/../mutils-containers -I$(PWD)/../mutils-serialization -I$(PWD)/../mutils-tasks -I$(PWD)/../testing -I$(PWD)/pgsql -I$(PWD)/../tracker -g -stdlib=libc++ --std=c++1z -Wall -DNUM_CAUSAL_GROUPS="4" -DCAUSAL_GROUP=$(causalGroup) -DSTRONG_REMOTE_IP=\"$(STRONG_REMOTE_IP)\" -DCAUSAL_REMOTE_IP_1=\"$(CAUSAL_REMOTE_IP_1)\" -DCAUSAL_REMOTE_IP_2=\"$(CAUSAL_REMOTE_IP_2)\" -DSTORE_LIST=$(storeList) -DMY_IP=\"$(MY_IP)\" -DMAX_THREADS=$(MAX_THREADS) -ferror-limit=1 -Wall -Werror -Wextra -DNUM_CAUSAL_MASTERS="2" -DNDEBUG
LDFLAGS= -stdlib=libc++ --std=c++1z -lpq -lm -pthread 
object_files=ServerSocket.o SerializationSupport.o utils.o Socket.o batched_connection_server.o batched_connection_client.o batched_connection_common.o SQLConnection.o LocalSQLConnection.o eventfd.o epoll.o oid_translator.o pgtransaction.o pgexceptions.o pgresult.o pgdeferred.o dual_connection.o
export IP_quad
export TRANS_DIR
export CPPFLAGS
export LDFLAGS

all:
	echo "don't use this target"
simple_test: $(object_files)
	clang++ basic_sql_test.cpp $(object_files) $(CPPFLAGS) $(LDFLAGS) -lpqxx

strong: $(object_files)
	clang++ LocalSQLReceiver.cpp $(object_files) -o strong_receiver -DRCVR_LEVEL=Level::strong $(CPPFLAGS) $(LDFLAGS)

causal: $(object_files)
	clang++ LocalSQLReceiver.cpp $(object_files) -o causal_receiver -DRCVR_LEVEL=Level::causal $(CPPFLAGS) $(LDFLAGS)

test_causal: $(object_files)
	clang++ test_causal.cpp $(object_files) -o test_causal_receiver -DRCVR_LEVEL=Level::causal $(CPPFLAGS) $(LDFLAGS)

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

batched_connection_server.o:
	clang++ -c -O3 ../*/batched_connection_server.cpp $(CPPFLAGS)

simple_rpc.o:
	clang++ -c ../*/simple_rpc.cpp $(CPPFLAGS)

eventfd.o:
	clang++ -c ../*/eventfd.cpp $(CPPFLAGS)

epoll.o:
	clang++ -c ../*/epoll.cpp $(CPPFLAGS)

dual_connection.o:
	clang++ -c -O3  ../*/dual_connection.cpp $(CPPFLAGS)

batched_connection_client.o:
	clang++ -c -O3 ../*/batched_connection_client.cpp $(CPPFLAGS)

batched_connection_common.o:
	clang++ -c -O3 ../*/batched_connection_common.cpp $(CPPFLAGS)

ServerSocket.o:
	clang++ -c ../*/ServerSocket.cpp $(CPPFLAGS)
SQLConnection.o:
	clang++ -c ../*/SQLConnection.cpp $(CPPFLAGS)

Socket.o:
	clang++ -c ../*/Socket.cpp $(CPPFLAGS)

SerializationSupport.o:
	clang++ -c ../*/SerializationSupport.cpp $(CPPFLAGS)

GlobalPool.o:
	clang++ -c ../*/GlobalPool.cpp $(CPPFLAGS)
proxy_connection.o:
	clang++ -c ../*/proxy_connection.cpp $(CPPFLAGS)
utils.o:
	clang++ -c ../*/utils.cpp $(CPPFLAGS)
clean:
	rm *.o; rm strong_receiver; rm causal_receiver;
