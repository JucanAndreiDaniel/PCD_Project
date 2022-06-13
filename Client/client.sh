#!/bin/bash
for i in {1..100};
do
    ./client Makefile 0 | ./client Makefile 2
done