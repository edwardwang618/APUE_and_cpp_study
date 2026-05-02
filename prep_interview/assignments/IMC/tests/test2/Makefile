CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic

SRCS = GameRules.cc Game.cc
OBJS = $(SRCS:.cc=.o)

all: psr_demo psr_tests

psr_demo: main.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

psr_tests: tests.o GameRules.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

test: psr_tests
	./psr_tests

clean:
	rm -f *.o psr_demo psr_tests

.PHONY: all test clean