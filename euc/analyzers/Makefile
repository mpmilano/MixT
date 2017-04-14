TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
storeList='pgsql::SQLStore<pgsql::Level::strong>, pgsql::SQLStore<pgsql::Level::causal>'
CPPFLAGS= -I$(PWD)/../../transactions/ -I$(PWD)/../../transactions//mtl -I$(PWD)/../../transactions//myria-utils -I$(PWD)/../../transactions//mutils-networking -I$(PWD)/../../transactions//mutils -I$(PWD)/../../transactions//mutils-containers -I$(PWD)/../../transactions//mutils-serialization -I$(PWD)/../../transactions//mutils-tasks -I$(PWD)/../../transactions//testing -I$(PWD)/../../transactions//server -I$(PWD)/../../transactions//pgsql -I$(PWD)/../../transactions//tracker -g -stdlib=libc++ --std=c++1z -DNUM_CAUSAL_GROUPS="4" -DSTORE_LIST=$(storeList) -DMY_IP=\"$(MY_IP)\" -DMAX_THREADS=$(MAX_THREADS) -DIP_QUAD=$(IP_quad) -DSTRONG_REMOTE_IP=\"$(STRONG_REMOTE_IP)\" -DCAUSAL_GROUP=$(causalGroup) -DCAUSAL_REMOTE_IP_1=\"$(CAUSAL_REMOTE_IP_1)\" -DCAUSAL_REMOTE_IP_2=\"$(CAUSAL_REMOTE_IP_2)\" -ferror-limit=1 -Wall -Werror -Wextra -DTRACK -DNDEBUG 
LDFLAGS= -stdlib=libc++ --std=c++1z -lm -pthread -lpqxx
object_files=utils.o SQLLevels.o

all: $(object_files) 
	clang++ main.cpp $(object_files) $(CPPFLAGS) -o main $(LDFLAGS)
utils.o:
	clang++ -c ../../transactions/*/utils.cpp $(CPPFLAGS)
SQLLevels.o:
	clang++ -c ../../transactions/*/SQLLevels.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)

clean:
	rm *.o
