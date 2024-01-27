#!/bin/bash

# zabicie wszystkich procesow o nazwie lot_ao.out
pkill -9 lot_ao.out

# kompilacja programu lot.c do pliku wykonywalnego lot_ao.out
gcc lot.c -o lot_ao.out -lpthread
