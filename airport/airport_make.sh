#!/bin/bash

# Kill all processes named airport.out
pkill -9 airport.out

# Compile airport.c to the executable airport.out
gcc airport.c -o airport.out -lpthread