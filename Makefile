CXX = g++
CXXFLAGS = -Wall -g -framework IOKit

ARCH := $(shell uname -m)
ifeq ($(ARCH), x86_64)
	TARGET_ARCH := $(ARCH)
else ifeq ($(ARCH), arm64)
	TARGET_ARCH := $(ARCH)
else
  $(error Not support architecture: $(ARCH))
endif
CXXFLAGS += -DTARGET_ARCH=$(TARGET_ARCH)

all: smctemp

smctemp: smctemp.o
	$(CXX) $(CXXFLAGS) -o smctemp smctemp.o

smctemp.o: smctemp.h smctemp.cc
	$(CXX) $(CXXFLAGS) -c smctemp.cc

.PHONY: clean

clean:
	-rm -f smctemp smctemp.o
