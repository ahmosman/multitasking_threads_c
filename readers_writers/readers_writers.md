# Solution Concept for the Readers and Writers Problem

## Problem Description

1. A fixed number of processes - N.
2. Processes alternate between two phases: relaxation and using the library.
3. In the relaxation phase, a process can change its role: from writer to reader or from reader to writer.
4. When entering the library phase, the process must obtain access in the appropriate mode.
5. A writer places their work in the message queue, where it remains until all readers who were in the reader role at the time of publication have read it.
6. The capacity of the message queue (the bookshelf) is limited to K.

## Solution Structure

### Shared Memory

- Shared memory segment with identifier 1234 containing 4 int values (mem_buf).

### Message Queues

- books_msgid queue for sending information about books.
- events_msgid queue for events related to readers.

### Semaphore

- Semaphore array (semid) for synchronizing access to shared resources.

## Relaxation and Library Phases

- Each process runs in an infinite loop, alternating between relaxation and library phases.
- Processes randomly change roles in the relaxation phase and wait for a random time.
- In the library phase, processes obtain access in the appropriate mode and perform the required actions.

## Constraints

- The books_msgid message queue represents the bookshelf, and its capacity is limited to K.
- The writer process places a work in the queue until it is read by all processes that were in the reader role at the time of publication.

## Synchronization and Access Control

- The semid semaphore is used to control access to shared resources.

## Additional Elements

- Helper functions: performSrand, sleepRandomSeconds, getRandomRole.
- Questions about implementation details and commands for monitoring/removing system resources.