// 1. There is a fixed number of processes — N
// 2. Each process alternates between two phases: relaxation and using the library;
// 3. At any time during the relaxation phase, a process may (but does not have to) change its role: from writer to reader or from reader to writer;
// 4. When entering the library phase, the process must obtain access in the appropriate mode for its current role;
// 5. A writer places the result of their work — their "book" — as a message in the message queue, where it remains until all processes that were readers at the time of publication have read it [they do not have to be in the library at that moment]
// (after all required processes have read it, the message is removed);
// 6. The capacity of the message queue — representing the bookshelf — is limited, i.e., it cannot store more than K works
// 7. While in the library, a process (including a writer) reads at most one work, after which the reader leaves the library, and the writer waits for space in the queue to publish another work.

// message queue - bookshelf with books
// semaphores - writing to the shelf
// memory

// Questions
// Does the process live indefinitely? - Yes
// Can there be a sleep when entering the relaxation phase? - Yes, a random one, then a role change and another sleep
// How to limit the capacity of the message queue? - not directly possible, must use shared memory

// Several readers enter the library
// Then the writer cannot enter
// When the readers leave, one writer enters
// If there is something to read (from the previous writer), they read it and then publish their own work before returning to relaxation

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <time.h>

#define ID 1234
#define ETYPE 1

static struct sembuf buf;

void semup(int semid, int semnum)
{
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1)
    {
        perror("ERR Raising semaphore");
        exit(1);
    }
}

void semdown(int semid, int semnum)
{
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1)
    {
        perror("ERR Lowering semaphore");
        exit(1);
    }
}

struct buf_books
{
    long btype;
    char btext[200];
    int brest_to_read;
};

struct buf_events
{
    long etype;
    long book_pid;
    int erest_to_read;
};

void performSrand()
{
    srand(time(NULL) ^ (getpid() << 16));
}

void sleepRandomSeconds()
{
    performSrand();

    int randomMicroseconds = rand() % (2500000) + 500000;

    int seconds = randomMicroseconds / 1000000;

    sleep(seconds);
}

int getRandomRole()
{
    performSrand();

    double r = (double)rand() / RAND_MAX; // Generate a number in the range [0, 1)

    if (r < 0.5)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int main()
{
    int shmid, semid, i, books_msgid, events_msgid, f;

    struct buf_books books;
    struct buf_events events;
    int *mem_buf;

    int N = 15;
    int K = 10;

    // role - 0 reader, 1 writer
    int role = 0;

    shmid = shmget(1234, 4 * sizeof(int), IPC_CREAT | 0600);
    if (shmid == -1)
    {
        perror("ERR Error creating shared memory");
        exit(1);
    }

    mem_buf = (int *)shmat(shmid, NULL, 0);
    if (mem_buf == NULL)
    {
        perror("ERR Attaching shared memory segment");
        exit(1);
    }

    books_msgid = msgget(ID, IPC_CREAT | IPC_EXCL | 0600);
    if (books_msgid == -1)
    {
        books_msgid = msgget(ID, IPC_CREAT | 0600);
        if (books_msgid == -1)
        {
            perror("ERR Creating books queue");
            exit(1);
        }
    }

    events_msgid = msgget(3548, IPC_CREAT | IPC_EXCL | 0600);
    if (events_msgid == -1)
    {
        events_msgid = msgget(ID, IPC_CREAT | 0600);
        if (events_msgid == -1)
        {
            perror("ERR Creating events queue");
            exit(1);
        }
    }

    semid = semget(ID, 5, IPC_CREAT | IPC_EXCL | 0600);
    if (semid == -1)
    {
        semid = semget(ID, 5, 0600);
        if (semid == -1)
        {
            perror("ERR Creating semaphore array");
            exit(1);
        }
    }
    else
    {
        if (semctl(semid, 0, SETVAL, (int)1) == -1) // number of readers
        {
            perror("ERR Setting value for semaphore 0");
            exit(1);
        }
        if (semctl(semid, 1, SETVAL, (int)1) == -1) // number of books on the shelf
        {
            perror("ERR Setting value for semaphore 1");
            exit(1);
        }
        if (semctl(semid, 2, SETVAL, (int)1) == -1)
        {
            perror("ERR Setting value for semaphore 2");
            exit(1);
        }
        if (semctl(semid, 3, SETVAL, (int)1) == -1)
        {
            perror("ERR Setting value for semaphore 3");
            exit(1);
        }
        if (semctl(semid, 4, SETVAL, (int)1) == -1)
        {
            perror("ERR Setting value for semaphore 4");
            exit(1);
        }
    }

    // Removing message queues and semaphore
    // msgctl(books_msgid, IPC_RMID, NULL);
    // msgctl(events_msgid, IPC_RMID, NULL);
    // semctl(semid, 0, IPC_RMID);

    mem_buf[0] = 0; // number of readers in the library
    mem_buf[1] = 0; // number of books on the shelf
    mem_buf[2] = 0; // number of writers in the library
    mem_buf[3] = 0; // number of readers outside the library
    mem_buf[4] = 0; // book number to be read by the writer

    int events_msg_size = sizeof(events) - sizeof(long);
    int books_msg_size = sizeof(books) - sizeof(long);
    // start N processes
    for (i = 0; i < N; i++)
    {
        f = fork();
        if (f == 0)
        {
            printf("Created process %d\n", getpid());
            while (1)
            {
                // relaxation phase - randomly set role

                sleep(1);
                performSrand();

                role = getRandomRole();

                sleepRandomSeconds();

                if (role == 0)
                {
                    printf("Process with PID %d is a reader\n", getpid());
                }
                else
                {
                    printf("Process with PID %d is a writer\n", getpid());
                }

                int readers_num;
                int writers_num;

                semdown(semid, 0);
                readers_num = mem_buf[0];
                semup(semid, 0);

                semdown(semid, 2);
                writers_num = mem_buf[2];
                semup(semid, 2);

                // wait for event
                if (role == 0)
                {
                    semdown(semid, 3);
                    mem_buf[3]++; // number of readers waiting for permission to enter the library
                    semup(semid, 3);

                    printf("Reader with PID %d is waiting for event\n", getpid());
                    if (msgrcv(events_msgid, &events, events_msg_size, ETYPE, 0) == -1)
                    {
                        perror("ERR Receiving event message");
                        exit(1);
                    }
                    printf("Reader with PID %d received event of adding book: %lu. Sent to %d recipients\n", getpid(), events.book_pid, events.erest_to_read);

                    semdown(semid, 3);
                    mem_buf[3]--;
                    printf("Waiting for permission: %d readers\n", mem_buf[3]);
                    semup(semid, 3);

                    events.erest_to_read--;

                    if (events.erest_to_read > 0)
                    {
                        if (msgsnd(events_msgid, &events, events_msg_size, 0) == -1)
                        {
                            perror("ERR Passing event message further");
                        }
                        printf("Entry event passed to %d readers\n", events.erest_to_read);
                    }
                }

                // library usage phase
                if (role == 0)
                {
                    // reader
                    semdown(semid, 2); // semaphore blocking entry when there is a writer
                    semdown(semid, 0); // number of readers in the library
                    mem_buf[0]++;
                    printf("Number of readers in the library: %d After entry by PID %d\n", mem_buf[0], getpid());
                    semup(semid, 0);
                    semup(semid, 2);

                    // read the book
                    if (msgrcv(books_msgid, &books, books_msg_size, events.book_pid, 0) == -1)
                    {
                        perror("ERR Receiving book message");
                    }
                    printf("Reader with PID %d read book: %s\n", getpid(), books.btext);

                    books.brest_to_read--;
                    if (books.brest_to_read > 0)
                    {
                        if (msgsnd(books_msgid, &books, books_msg_size, 0) == -1)
                        {
                            perror("ERR Passing book message further");
                        }
                    }
                    else
                    {
                        // book read by everyone, remove from queue (shelf)
                        semdown(semid, 1);
                        mem_buf[1]--;
                        semup(semid, 1);

                        semdown(semid, 4);
                        if (mem_buf[4] == events.book_pid) // book is no longer available to be read by the writer
                        {
                            mem_buf[4] = 0;
                        }
                        semup(semid, 4);
                    }

                    semdown(semid, 0);
                    mem_buf[0]--;
                    printf("Reader with PID %d is leaving the library\n", getpid());
                    printf("Number of readers: %d After leaving by PID %d\n", mem_buf[0], getpid());
                    semup(semid, 0);
                }
                else if (role == 1)
                {
                    // writer

                    semdown(semid, 2);
                    mem_buf[2] = 1;
                    printf("Number of writers in the library: %d After entry by writer PID %d\n", mem_buf[2], getpid());

                    int book_to_read;
                    semdown(semid, 4);
                    book_to_read = mem_buf[4];

                    if (book_to_read > 0)
                    {
                        // writer reads the book
                        printf("Writer with ID %d wants to read book %d\n", getpid(), book_to_read);
                        if (msgrcv(books_msgid, &books, books_msg_size, book_to_read, 0) == -1)
                        {
                            perror("ERR Receiving book message");
                        }
                        printf("Writer with PID %d read book: %s\n", getpid(), books.btext);
                        printf("After reading the book by writer PID %d, %d readers remain to read\n", getpid(), books.brest_to_read);
                        if (books.brest_to_read > 0)
                        {
                            if (msgsnd(books_msgid, &books, books_msg_size, 0) == -1)
                            {
                                perror("ERR Passing book message further by writer");
                            }
                        }
                        else
                        {
                            // book read by everyone, remove from queue (shelf)
                            semdown(semid, 1);
                            mem_buf[1]--;
                            semup(semid, 1);

                            mem_buf[4] = 0;
                        }
                    }

                    semup(semid, 4);

                    performSrand();

                    int book_number = rand() % 1000;

                    int books_on_shelf;
                    semdown(semid, 1);
                    books_on_shelf = mem_buf[1];
                    semup(semid, 1);

                    if (books_on_shelf < K)
                    {
                        books.btype = book_number;

                        semdown(semid, 3);
                        books.brest_to_read = mem_buf[3];
                        events.erest_to_read = mem_buf[3];
                        semup(semid, 3);

                        sprintf(books.btext, "Book no. %lu written by writer with PID %d", books.btype, getpid());
                        if (msgsnd(books_msgid, &books, books_msg_size, 0) == -1)
                        {
                            perror("ERR Sending new book");
                        }

                        semdown(semid, 1);
                        mem_buf[1]++;

                        printf("Number of books on the shelf: %d After addition by writer PID %d\n", mem_buf[1], getpid());

                        semup(semid, 1);

                        semdown(semid, 4);
                        mem_buf[4] = books.btype;
                        semup(semid, 4);

                        if (events.erest_to_read > 0)
                        {
                            events.etype = ETYPE;
                            events.book_pid = books.btype;
                            if (msgsnd(events_msgid, &events, events_msg_size, 0) == -1)
                            {
                                perror("ERR Sending event message about new book addition");
                            }
                            printf("Writer with PID %d sent event of adding book %d to %d recipients\n", getpid(), book_number, events.erest_to_read);
                        }
                    }
                    else
                    {
                        printf("Writer with PID %d has no space on the shelf\n", getpid());
                    }

                    mem_buf[2] = 0;
                    printf("Number of writers: %d After leaving by writer PID %d\n", mem_buf[2], getpid());
                    semup(semid, 2);
                }
            }
        }
    }
}