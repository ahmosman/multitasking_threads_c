#!/bin/bash

# pkill -9 readers_writers.out - kills all processes named readers_writers.out
pkill -9 readers_writers.out

# ipcs - displays all semaphores, message queues, and shared memory
# ipcs

# ipcrm -a - removes all semaphores, message queues, and shared memory
ipcrm -a

# gcc readers_writers.c -o readers_writers.out
gcc readers_writers.c -o readers_writers.out