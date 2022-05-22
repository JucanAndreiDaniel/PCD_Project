################################################################################
#
#      Copyright (C) 2017 by Intel Corporation, All Rights Reserved.
#
#            Global configuration Makefile. Included everywhere.
#
################################################################################

# EDIT HERE:
CC:=gcc
CFLAGS:= -lcrypto -lssl -lpthread
ENABLE_TESTS=false

# override MinGW built-in recipe
%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

ifeq ($(OS),Windows_NT)
DOTEXE:=.exe
endif

# DO NOT EDIT AFTER THIS POINT:
ifeq ($(ENABLE_TESTS), true)
CFLAGS += -DENABLE_TESTS
else
CFLAGS += -DDISABLE_TESTS
endif

export CC
export CFLAGS
export ENABLE_TESTS

################################################################################
