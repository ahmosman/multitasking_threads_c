# Multitasking & Threads C Project

![Release Date](https://img.shields.io/badge/release-January%202024-blue)

This project contains C implementations of classic synchronization problems using threads and inter-process communication mechanisms. The code is organized into modules, each demonstrating a different concurrency scenario.

## Modules

### 1. Airport (Airplane Synchronization)
- **Location:** [`airport/`](airport/)
- **Description:** Simulates planes landing and taking off on an aircraft carrier with limited capacity and priority rules for landing. Uses POSIX threads, mutexes, and condition variables for synchronization.
- **Files:**
  - [`airport.c`](airport/airport.c): Source code
  - [`airport.md`](airport/airport.md): Problem description and solution concept
  - [`airport_make.sh`](airport/airport_make.sh): Build script

### 2. Readers-Writers
- **Location:** [`readers_writers/`](readers_writers/)
- **Description:** Implements the readers-writers problem with message queues, semaphores, and shared memory. Demonstrates process synchronization and resource sharing.
- **Files:**
  - [`readers_writers.c`](readers_writers/readers_writers.c): Source code
  - [`readers_writers.md`](readers_writers/readers_writers.md): Problem description and solution concept
  - [`readers_writers_make.sh`](readers_writers/readers_writers_make.sh): Build script

## How to Build

Each module contains a shell script to build the corresponding executable. For example, to build the airport module:

```sh
cd airport
bash airport_make.sh
```
## Technologies Used
- C programming language
- POSIX threads for concurrency
- Inter-process communication (IPC) mechanisms: message queues, semaphores, and shared memory
- Mutexes and condition variables for synchronization

## License

This project is licensed under the [MIT License](LICENSE).

## Release

**Release Date:**