TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
storeList='pgsql::SQLStore<Level::strong>, pgsql::SQLStore<Level::causal>'
CPPFLAGS= -I$(PWD) -I$(PWD)/../ -I$(PWD)/../mtl -I$(PWD)/..//myria-utils -I$(PWD)/../mutils-networking -I$(PWD)/../mutils -I$(PWD)/../mutils-containers -I$(PWD)/../mutils-serialization -I$(PWD)/../mutils-tasks -I$(PWD)/../testing -I$(PWD)/pgsql -I$(PWD)/../tracker -g -stdlib=libc++ --std=c++1z -Wall -DNUM_CAUSAL_GROUPS="4" -DCAUSAL_GROUP=$(causalGroup) -DCAUSAL_REMOTE_IP_1=\"$(CAUSAL_REMOTE_IP_1)\" -DCAUSAL_REMOTE_IP_2=\"$(CAUSAL_REMOTE_IP_2)\" -DSTORE_LIST=$(storeList) -DMY_IP=\"$(MY_IP)\" -DMAX_THREADS=$(MAX_THREADS) -ferror-limit=1 -Wall -Werror -Wextra -DNUM_CAUSAL_MASTERS="2" -DNDEBUG
LDFLAGS= -stdlib=libc++ --std=c++1z -lpqxx -lm -pthread
object_files=ServerSocket.o SerializationSupport.o utils.o Socket.o BlobUtils.o batched_connection_server.o batched_connection_client.o batched_connection_common.o SQLConnection.o
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

test_causal: $(object_files)
	clang++ test_causal.cpp $(object_files) -o test_causal_receiver -DRCVR_LEVEL=Level::causal $(CPPFLAGS) $(LDFLAGS)

batched_connection_server.o:
	clang++ -c -O3 ../*/batched_connection_server.cpp $(CPPFLAGS)

simple_rpc.o:
	clang++ -c ../*/simple_rpc.cpp $(CPPFLAGS)

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
BlobUtils.o:
	clang++ -c ../*/BlobUtils.cpp $(CPPFLAGS)
clean:
	rm *.o; rm strong_receiver; rm causal_receiver;
