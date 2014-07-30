all:
	clang++ Votes.cpp Client.cpp --std=c++1y -Wall -Wextra -Wno-missing-braces  -g -o votes

test:
	clang++ frontend-test.cpp --std=c++1y -Wall -Wextra -Wno-missing-braces  -g
