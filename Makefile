CXX = g++
CXXFLAGS = -Wall -std=c++17 -g -framework IOKit
EXES = smctemp
DEST = /usr/local/bin

ARCH := $(shell uname -m)
PROCESS_IS_TRANSLATED := $(shell sysctl -in sysctl.proc_translated)
ifeq ($(ARCH), x86_64)
ifeq ($(PROCESS_IS_TRANSLATED), 1)
	# Running under Rosetta
	CXXFLAGS += -DARCH_TYPE_ARM64
else
	CXXFLAGS += -DARCH_TYPE_X86_64
endif
else ifeq ($(ARCH), arm64)
	CXXFLAGS += -DARCH_TYPE_ARM64
else
	$(error Not support architecture: $(ARCH))
endif

all: $(EXES)

$(EXES): smctemp_string.o smctemp.o main.cc
	$(CXX) $(CXXFLAGS) -o $(EXES) smctemp.o smctemp_string.o main.cc

smctemp.o: smctemp_string.h smctemp.h smctemp.cc
	$(CXX) $(CXXFLAGS) -o smctemp.o -c smctemp.cc

smctemp_string.o: smctemp_string.h smctemp_string.cc
	$(CXX) $(CXXFLAGS) -o smctemp_string.o -c smctemp_string.cc

install: $(EXES)
	install -d $(DEST)
	install -m 0755 $(EXES) $(DEST)

clean:
	rm -rf $(EXES) smctemp.o smctemp_string.o smctemp.dSYM

.PHONY: clean
