TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
storeList='pgsql::SQLStore<Level::strong>, pgsql::SQLStore<Level::causal>'
CPPFLAGS= -I$(PWD) -I$(PWD)/../ -I$(PWD)/../mtl -I$(PWD)/..//myria-utils -I$(PWD)/../mutils-networking -I$(PWD)/../mutils -I$(PWD)/../mutils-containers -I$(PWD)/../mutils-serialization -I$(PWD)/../mutils-tasks -I$(PWD)/../testing -I$(PWD)/pgsql -I$(PWD)/../tracker -g -stdlib=libc++ --std=c++1z -Wall -DNUM_CAUSAL_GROUPS="4" -DCAUSAL_GROUP=$(causalGroup) -DSTORE_LIST=$(storeList) -DMY_IP=\"$(MY_IP)\" -DMAX_THREADS=$(MAX_THREADS) -ferror-limit=1 -Wall -Werror -Wextra -DNUM_CAUSAL_MASTERS="2"
LDFLAGS= -stdlib=libc++ --std=c++1z -lpqxx -lm -pthread
object_files=LocalSQLReceiver.o
export IP_quad
export TRANS_DIR
export CPPFLAGS
export LDFLAGS

all:
	echo "don't use this target"

strong:
	clang++ LocalSQLReceiver.cpp -o strong_receiver -DRCVR_LEVEL=Level::strong $(CPPFLAGS) $(LDFLAGS)

causal:
	clang++ LocalSQLReceiver.cpp -o causal_receiver -DRCVR_LEVEL=Level::causal $(CPPFLAGS) $(LDFLAGS)

clean:
	rm strong_receiver; rm causal_receiver;
