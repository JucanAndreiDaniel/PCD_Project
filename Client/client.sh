#!/bin/bash
for i in {1..10}; 
do 
    gcc client.c -o client && ./client; 
done