CC = g++
CCFLAGS = -std=c++11 -Wall -O3 -g -DNDEBUG -pthread
LDFLAGS = -lpthread -pthread
LDLIBUV = -luv -Wl,-rpath=/usr/local/lib

EXECUTABLES = \
	server

all: $(EXECUTABLES)

server: utils.c main.cpp
	$(CC) $(CCFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBUV)

.PHONY: clean

clean:
	rm -f $(EXECUTABLES) *.o
