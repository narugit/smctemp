CXX := g++
CXXFLAGS := -Wall -std=c++17 -g -framework IOKit
EXES := smctemp
STATIC_LIB := libsmctemp.a
DEST_PREFIX := /usr/local
AR := ar
ARFLAGS := rc
RANLIB := ranlib

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

OBJS := smctemp.o \
        smctemp_string.o

HEADERS := smctemp.h \
           smctemp_string.h \
           smctemp_types.h

all: $(EXES)

$(EXES): $(OBJS) main.cc
	$(CXX) $(CXXFLAGS) -o $(EXES) $(OBJS) main.cc

staticlib: $(OBJS)
	$(RM) $(STATIC_LIB)
	$(AR) $(ARFLAGS) $(STATIC_LIB) $^
	$(RANLIB) $(STATIC_LIB)

smctemp.o: smctemp_string.h smctemp.h smctemp.cc
	$(CXX) $(CXXFLAGS) -o smctemp.o -c smctemp.cc

smctemp_string.o: smctemp_string.h smctemp_string.cc
	$(CXX) $(CXXFLAGS) -o smctemp_string.o -c smctemp_string.cc

install: $(EXES)
	install -d $(DEST_PREFIX)/bin
	install -m 0755 $(EXES) $(DEST_PREFIX)/bin

installstaticlib: $(STATIC_LIB)
	install -d $(DEST_PREFIX)/lib
	install -d $(DEST_PREFIX)/include
	install -m 0644 $(STATIC_LIB) $(DEST_PREFIX)/lib
	install -m 0644 $(HEADERS) $(DEST_PREFIX)/include

clean:
	$(RM) -r $(EXES) smctemp.o smctemp_string.o smctemp.dSYM $(STATIC_LIB)

.PHONY: clean
