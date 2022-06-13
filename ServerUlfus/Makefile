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

all: serverV2

clean:
	rm -f *.o serverV2

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: serverV2


static_compressed_inmemory_website_callback.o: $(INCLUDE)/static_compressed_inmemory_website_callback.c
	$(CC) $(CFLAGS) $(INCLUDE)/static_compressed_inmemory_website_callback.c

http_compression_callback.o: $(INCLUDE)/http_compression_callback.c
	$(CC) $(CFLAGS) $(INCLUDE)/http_compression_callback.c

serverV2.o: serverV2.c
	$(CC) $(CFLAGS) serverV2.c -DDEBUG -g -O0

serverV2: serverV2.o static_compressed_inmemory_website_callback.o http_compression_callback.o
	$(CC) -o serverV2 serverV2.o static_compressed_inmemory_website_callback.o http_compression_callback.o $(LIBS)

test: serverV2
	LD_LIBRARY_PATH=$(INCLUDE):${LD_LIBRARY_PATH} ./serverV2
