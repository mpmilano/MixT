TRANS_DIR= `pwd`/../
clangFlags= -Xclang -fcolor-diagnostics -g -stdlib=libc++ --std=c++14 -Wall -ferror-limit=1	
linkFlags= -stdlib=libc++ --std=c++14 -lpqxx
export TRANS_DIR
export clangFlags
export linkFlags

all: 
	clang++ sync.cpp -o sync $(linkFlags)
