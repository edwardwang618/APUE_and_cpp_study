CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2

.PHONY: all clean test

all: visitor_demo test_runner

visitor_demo: main.o AreaVisitor.o PerimeterVisitor.o
	$(CXX) $(CXXFLAGS) -o $@ $^

test_runner: tests.o AreaVisitor.o PerimeterVisitor.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

test: test_runner
	./test_runner

clean:
	rm -f *.o visitor_demo test_runner