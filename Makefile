all:
	clang++ Votes.cpp -stdlib=libc++ --std=c++1y -Wall -Wextra -g -o votes

test:
	clang++ frontend-test.cpp -stdlib=libc++ --std=c++1y -Wall -Wextra -g
