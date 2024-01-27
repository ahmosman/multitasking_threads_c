#!/bin/bash


# pkill -9 czyp_ao.out - zabija wszystkie procesy o nazwie czyp_ao.out
pkill -9 czyp_ao.out

# ipcs - wyswietla wszystkie semafory, kolejki komunikatow i pamieci wspoldzielone
# ipcs

# ipcrm -a - usuwa wszystkie semafory, kolejki komunikatow i pamieci wspoldzielone
ipcrm -a


# gcc czypis.c -o czyp_ao.out
gcc czypis.c -o czyp_ao.out