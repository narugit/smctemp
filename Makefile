CXX = g++
CXXFLAGS = -Wall -g -framework IOKit

all: smctemp

smctemp: smctemp.o
	$(CXX) $(CXXFLAGS) -o smctemp smctemp.o

smctemp.o: smctemp.h smctemp.cc
	$(CXX) $(CXXFLAGS) -c smctemp.cc

.PHONY: clean

clean:
	-rm -f smctemp smctemp.o
