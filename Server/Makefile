#
# Sheep counter program
#
# Makefile used to build the software
#
# Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
CC=gcc
INCLUDE=./include
CFLAGS+=-c -Wall -I$(INCLUDE) -D_REENTRANT $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-lc -lz -lpthread -lulfius -lorcania -ljansson -lcrypto -lssl 

ifndef YDERFLAG
LIBS+= -lyder
endif

all: server

clean:
	rm -f *.o server

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

serverINET.o: serverINET.c
	$(CC) $(CFLAGS) serverINET.c

serverUNIX.o: serverUNIX.c
	$(CC) $(CFLAGS) serverUNIX.c

serverREST.o: serverREST.c
	$(CC) $(CFLAGS) serverREST.c -DDEBUG -g -O0

http_compression_callback.o: $(INCLUDE)/http_compression_callback.c
	$(CC) $(CFLAGS) $(INCLUDE)/http_compression_callback.c

checksum.o: $(INCLUDE)/checksum.c
	$(CC) $(CFLAGS) $(INCLUDE)/checksum.c

server.o: server.c
	$(CC) $(CFLAGS) server.c 

server: server.o serverUNIX.o serverINET.o serverREST.o http_compression_callback.o checksum.o
	$(CC) -o server server.o serverUNIX.o serverINET.o serverREST.o http_compression_callback.o checksum.o $(LIBS)

test: server
	LD_LIBRARY_PATH=$(INCLUDE):${LD_LIBRARY_PATH} ./server
