all:
	clang++ Votes.cpp --std=c++1y -Wall -Wextra -Wno-missing-braces  -g -o votes

test:
	clang++ frontend-test.cpp --std=c++1y -Wall -Wextra -Wno-missing-braces  -g
