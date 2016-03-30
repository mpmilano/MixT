TRANS_DIR= `pwd`/../
clangFlags= -Xclang -fcolor-diagnostics -g -stdlib=libc++ --std=c++14 -Wall -ferror-limit=1	
linkFlags= -stdlib=libc++ --std=c++14 -lpqxx
export TRANS_DIR
export clangFlags
export linkFlags

all: sync print_strong print_causal
	echo "done"

sync:
	clang++ $(clangFlags) sync.cpp -o sync $(linkFlags)

print_strong:
	clang++ $(clangFlags) print_strong.cpp -o print_strong $(linkFlags)

print_causal:
	clang++ $(clangFlags) print_causal.cpp -o print_causal $(linkFlags)
