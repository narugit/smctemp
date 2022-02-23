CXX = g++
CXXFLAGS = -Wall -std=c++20 -g -framework IOKit

ARCH := $(shell uname -m)
ifeq ($(ARCH), x86_64)
	CXXFLAGS += -DARCH_TYPE_X86_64
else ifeq ($(ARCH), arm64)
	CXXFLAGS += -DARCH_TYPE_ARM64
else
  $(error Not support architecture: $(ARCH))
endif

all: smctemp

smctemp: smctemp_string.o smctemp.o main.cc
	$(CXX) $(CXXFLAGS) -o smctemp smctemp.o smctemp_string.o main.cc

smctemp.o: smctemp_string.h smctemp.h smctemp.cc
	$(CXX) $(CXXFLAGS) -o smctemp.o -c smctemp.cc

smctemp_string.o: smctemp_string.h smctemp_string.cc
	$(CXX) $(CXXFLAGS) -o smctemp_string.o -c smctemp_string.cc

.PHONY: clean

clean:
	-rm -f smctemp smctemp.o smctemp_string.o
