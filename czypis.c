// 1. jest ustalona liczba procesów — N
// 2 każdy proces działa naprzemiennie w dwóch fazach: fazie relaksu i fazie korzystania z czytelni;
// 3 w dowolnym momencie fazy relaksu proces może (choć nie musi) zmienić swoją rolę: z pisarza na czytelnika lub z czytelnika na pisarza;
// 4 przechodząc do fazy korzystania z czytelni proces musi uzyskać dostęp we właściwym dla swojej aktualnej roli trybie;
// 5 pisarz umieszcza efekt swojej pracy — swoje dzieło — w formie komunikatu w kolejce komunikatów, gdzie komunikat ten pozostaje do momentu,
// aż odczytają go wszystkie procesy, które w momencie wydania dzieła były w roli czytelnika [nie muszą być akurat w tym momencie w czytelni]
// (po odczytaniu przez wszystkie wymagane procesy komunikat jest usuwany);
// 6 pojemność kolejki komunikatów — reprezentującej półkę z książkami — jest ograniczona, tzn. nie może ona przechowywać więcej niż K dzieł
// 7 podczas pobytu w czytelni proces (również pisarz) czyta co najwyżej jedno dzieło, po czym czytelnik opuszcza czytelnię, a pisarz czeka na miejsce w kolejce,
// żeby opublikować kolejne dzieło.

// kolejka komunikatow - półka z książkami
// semafory - zapis do półki
// pamięć

// Pytania
// Czy proces żyje nieskończenie długo? - Tak
// Czy przy przejściu w fazę relaksu może być sleep? - Tak, jeden losowy, potem zmiana roli i jakiś kolejny sleep
// Jak ograniczyc pojemnosc kolejki komunikatow? - nie da się bezpośrednio, trzeba pamięcia współdzieloną

// Do biblioteki wchodzi kilku czytelnikow
// Wtedy pisarz nie może wejsc
// Jak czytelnicy wychodzą to wchodzi jeden pisaz
// Wtedy jezeli coś jest do przeczytania ( od poprzedniego pisarza ) to czyta to i publikuje swoje dzieło po czym wychodzi do stanu relaksu

// Komendy
// ipcs - wyswietla wszystkie semafory, kolejki komunikatow i pamieci wspoldzielone
// ipcrm -a   - usuwa wszystkie semafory, kolejki komunikatow i pamieci wspoldzielone
// gcc czypis.c -o p.out
// pkill -9 p.out - zabija wszystkie procesy o nazwie p.out

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
        perror("ERR Podnoszenie semafora");
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
        perror("ERR Opuszczenie semafora");
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

    double r = (double)rand() / RAND_MAX; // Generuj liczbę z zakresu [0, 1)

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

    // role - 0 czytelnik, 1 pisarz
    int role = 0;

    shmid = shmget(1234, 4 * sizeof(int), IPC_CREAT | 0600);
    if (shmid == -1)
    {
        perror("ERR Blad tworzenia pamieci wspoldzielonej");
        exit(1);
    }

    mem_buf = (int *)shmat(shmid, NULL, 0);
    if (mem_buf == NULL)
    {
        perror("ERR Przylaczenie segmentu pamieci wspoldzielonej");
        exit(1);
    }

    books_msgid = msgget(ID, IPC_CREAT | IPC_EXCL | 0600);
    if (books_msgid == -1)
    {
        books_msgid = msgget(ID, IPC_CREAT | 0600);
        if (books_msgid == -1)
        {
            perror("ERR Utworzenie kolejki ksiazek");
            exit(1);
        }
    }

    events_msgid = msgget(3548, IPC_CREAT | IPC_EXCL | 0600);
    if (events_msgid == -1)
    {
        events_msgid = msgget(ID, IPC_CREAT | 0600);
        if (events_msgid == -1)
        {
            perror("ERR Utworzenie kolejki zdarzen");
            exit(1);
        }
    }

    semid = semget(ID, 5, IPC_CREAT | IPC_EXCL | 0600);
    if (semid == -1)
    {
        semid = semget(ID, 5, 0600);
        if (semid == -1)
        {
            perror("ERR Utworzenie tablicy semaforow");
            exit(1);
        }
    }
    else
    {
        if (semctl(semid, 0, SETVAL, (int)1) == -1) // liczba czytelników
        {
            perror("ERR Nadanie wartosci semaforowi 0");
            exit(1);
        }
        if (semctl(semid, 1, SETVAL, (int)1) == -1) // liczba ksiazek na polce
        {
            perror("ERR Nadanie wartosci semaforowi 1");
            exit(1);
        }
        if (semctl(semid, 2, SETVAL, (int)1) == -1)
        {
            perror("ERR Nadanie wartosci semaforowi 2");
            exit(1);
        }
        if (semctl(semid, 3, SETVAL, (int)1) == -1)
        {
            perror("ERR Nadanie wartosci semaforowi 3");
            exit(1);
        }
        if (semctl(semid, 4, SETVAL, (int)1) == -1)
        {
            perror("ERR Nadanie wartosci semaforowi 4");
            exit(1);
        }
    }

    // Usunięcie kolejki komunikatów i semafora
    // msgctl(books_msgid, IPC_RMID, NULL);
    // msgctl(events_msgid, IPC_RMID, NULL);
    // semctl(semid, 0, IPC_RMID);

    mem_buf[0] = 0; // liczba czytlenikow w czytelni
    mem_buf[1] = 0; // liczba ksiazek na polce
    mem_buf[2] = 0; // liczba pisarzy w czytelni
    mem_buf[3] = 0; // liczba czytelnikow poza czytelnia
    mem_buf[4] = 0; // numer ksiazki do odczytania przez pisarza

    int events_msg_size = sizeof(events) - sizeof(long);
    int books_msg_size = sizeof(books) - sizeof(long);
    // uruchom N procesow
    for (i = 0; i < N; i++)
    {
        f = fork();
        if (f == 0)
        {
            printf("Stworzono proces %d\n", getpid());
            while (1)
            {
                // faza relaksu - ustaw losowo role

                sleep(1);
                performSrand();

                role = getRandomRole();

                sleepRandomSeconds();

                if (role == 0)
                {
                    printf("Proces o PID %d jest czytelnikiem\n", getpid());
                }
                else
                {
                    printf("Proces o PID %d jest pisarzem\n", getpid());
                }

                int readers_num;
                int writers_num;

                semdown(semid, 0);
                readers_num = mem_buf[0];
                semup(semid, 0);

                semdown(semid, 2);
                writers_num = mem_buf[2];
                semup(semid, 2);

                // oczekujemy na event
                if (role == 0)
                {
                    semdown(semid, 3);
                    mem_buf[3]++; // liczba czytelnikow oczekujących na pozwolenie wejscia do czytelni
                    semup(semid, 3);

                    printf("Czytelnik o PID %d czeka na event\n", getpid());
                    if (msgrcv(events_msgid, &events, events_msg_size, ETYPE, 0) == -1)
                    {
                        perror("ERR Odebranie komunikatu o wydarzeniu");
                        exit(1);
                    }
                    printf("Czytelnik o PID %d dostal event dodania ksiazki: %d. Wyslany zostal do %d odbiorcow\n", getpid(), events.book_pid, events.erest_to_read);

                    semdown(semid, 3);
                    mem_buf[3]--;
                    printf("Oczekuje na pozwolenie: %d czytelnikow\n", mem_buf[3]);
                    semup(semid, 3);

                    events.erest_to_read--;

                    if (events.erest_to_read > 0)
                    {
                        if (msgsnd(events_msgid, &events, events_msg_size, 0) == -1)
                        {
                            perror("ERR ERR Przekazanie dalej komunikatu o wydarzeniu");
                        }
                        printf("Event wejscia przekazany do %d czytelnikow\n", events.erest_to_read);
                    }
                }

                // faza korzystania z czytelni
                if (role == 0)
                {
                    // czytelnik
                    semdown(semid, 2); // semafor blokujacy przed wejsciem gdy jest pisarz
                    semdown(semid, 0); // liczba czytlenikow w czytelni
                    mem_buf[0]++;
                    printf("Liczba czytelnikow w czytelni: %d Po wejsciu przez PID %d\n", mem_buf[0], getpid());
                    semup(semid, 0);
                    semup(semid, 2);

                    // odczytujemy ksiazke
                    if (msgrcv(books_msgid, &books, books_msg_size, events.book_pid, 0) == -1)
                    {
                        perror("ERR Odebranie komunikatu z ksiazka");
                    }
                    printf("Czytelnik o PID %d odczytal ksiazke: %s\n", getpid(), books.btext);

                    books.brest_to_read--;
                    if (books.brest_to_read > 0)
                    {
                        if (msgsnd(books_msgid, &books, books_msg_size, 0) == -1)
                        {
                            perror("ERR Przekazanie dalej komunikatu z ksiazka");
                        }
                    }
                    else
                    {
                        // ksiazka przeczytana przez wszystkich, usun z kolejki (polki)
                        semdown(semid, 1);
                        mem_buf[1]--;
                        semup(semid, 1);

                        semdown(semid, 4);
                        if (mem_buf[4] == events.book_pid) // ksiazka nie jest dostepna juz do odczytania przez pisarza
                        {
                            mem_buf[4] = 0;
                        }
                        semup(semid, 4);
                    }

                    semdown(semid, 0);
                    mem_buf[0]--;
                    printf("Czytelnik o PID %d opuszcza czytelnie\n", getpid());
                    printf("Liczba czytelnikow: %d Po opuszczeniu przez PID %d\n", mem_buf[0], getpid());
                    semup(semid, 0);
                }
                else if (role == 1)
                {
                    // pisarz

                    semdown(semid, 2);
                    mem_buf[2] = 1;
                    printf("Liczba pisarzy w czytelni: %d Po wejsciu przez pisarza PID %d\n", mem_buf[2], getpid());

                    int book_to_read;
                    semdown(semid, 4);
                    book_to_read = mem_buf[4];

                    if (book_to_read > 0)
                    {
                        // pisarz odczytuje ksiazke
                        printf("Pisarz o ID %d chce odczytac ksiazke %d\n", getpid(), book_to_read);
                        if (msgrcv(books_msgid, &books, books_msg_size, book_to_read, 0) == -1)
                        {
                            perror("ERR Odebranie komunikatu z ksiazka");
                        }
                        printf("Pisarz o PID %d odczytal ksiazke: %s\n", getpid(), books.btext);
                        printf("Po odczytaniu ksiazki przez pisarza PID %d zostalo %d czytelnikow do przeczytania\n", getpid(), books.brest_to_read);
                        if (books.brest_to_read > 0)
                        {
                            if (msgsnd(books_msgid, &books, books_msg_size, 0) == -1)
                            {
                                perror("ERR Przekazanie dalej komunikatu z ksiazka przez pisarza");
                            }
                        }
                        else
                        {
                            // ksiazka przeczytana przez wszystkich, usun z kolejki (polki)
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

                        sprintf(books.btext, "Ksiazka nr %d zapisana przez pisarza o PID %d", books.btype, getpid());
                        if (msgsnd(books_msgid, &books, books_msg_size, 0) == -1)
                        {
                            perror("ERR Wyslanie nowej ksiazki");
                        }

                        semdown(semid, 1);
                        mem_buf[1]++;

                        printf("Liczba ksiazek na polce: %d Po dodaniu przez pisarza PID %d\n", mem_buf[1], getpid());

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
                                perror("ERR Wyslanie komunikatu o nowym wydarzeniu dodania ksiazki");
                            }
                            printf("Pisarz o PID %d wyslal event dodania ksiazki %d do %d odbiorców\n", getpid(), book_number, events.erest_to_read);
                        }
                    }
                    else
                    {
                        printf("Pisarz o PID %d nie ma miejsca na polce\n", getpid());
                    }

                    mem_buf[2] = 0;
                    printf("Liczba pisarzy: %d Po wyjsciu przez pisarza PID %d\n", mem_buf[2], getpid());
                    semup(semid, 2);
                }
            }
        }
    }
}