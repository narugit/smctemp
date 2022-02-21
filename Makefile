CC = gcc
CFLAGS = -mmacosx-version-min=10.4  -Wall -g -framework IOKit
CPPFLAGS = -DCMD_TOOL_BUILD

all: smctemp

smctemp: smctemp.o
	$(CC) $(CFLAGS) -o smctemp smctemp.o

smctemp.o: smctemp.h smctemp.c
	$(CC) $(CPPFLAGS) -c smctemp.c

clean:
	-rm -f smctemp smctemp.o
