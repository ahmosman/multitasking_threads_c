// Na lotniskowcu lądują i startują samoloty. W tym celu potrzebują wyłącznego dostępu do pasa startowego.
// Lotniskowiec może pomieścić pewną ustaloną liczbę N samolotów. Jeśli liczba samolotów na lotniskowcu jest mniejsza
// niż K (K < N), wówczas priorytet w dostępie do pasa mają samoloty lądujące.

// Cel zadania: synchronizacja samolotów (pas i miejsce na lotniskowcu to zasoby)

// Źródło zadania: Weiss Z., Gruźlewski T., Programowanie współbieżne i rozproszone w przykładach i zadaniach. WNT W-wa 1993

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <stdbool.h>

static pthread_mutex_t runway_mtx = PTHREAD_MUTEX_INITIALIZER,
                       space_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t runway_available_cond = PTHREAD_COND_INITIALIZER,
                      space_available = PTHREAD_COND_INITIALIZER;

static int airport_num = 0, takeoff_waiting_num = 0, landing_waiting_num = 0;
static bool is_runway_available = true;

int K = 3;
int N = 5;

void sleepRandomMilliseconds(int from, int to)
{
    srand(time(NULL));

    int randomMilliseconds = rand() % (to - from + 1) + from;

    usleep(randomMilliseconds * 1000);
}

void *landing(void *p)
{
    int plane_id = *((int *)p);
    int curr_airport_num = airport_num;

    pthread_mutex_lock(&space_mtx);
    landing_waiting_num++;

    pthread_mutex_lock(&runway_mtx);
    curr_airport_num = airport_num;
    pthread_mutex_unlock(&runway_mtx);

    if (curr_airport_num >= N || landing_waiting_num > 1)
    {
        printf("Samolot o ID %d czeka na miejsce na lotnisku\n", plane_id);
        pthread_cond_wait(&space_available, &space_mtx);
    }
    pthread_mutex_unlock(&space_mtx);

    pthread_mutex_lock(&runway_mtx);
    if (!is_runway_available)
    {
        printf("Samolot o ID %d czeka na pas do ladowania\n", plane_id);
        pthread_cond_wait(&runway_available_cond, &runway_mtx);
    }
    is_runway_available = false;
    printf("Ladowanie samolotu o ID %d\n", plane_id);
    sleepRandomMilliseconds(500, 2000);
    airport_num++;
    is_runway_available = true;
    landing_waiting_num--;

    printf("Liczba samolotow na lotnisku po ladowaniu: %d\n", airport_num);
    pthread_mutex_unlock(&runway_mtx);
    pthread_cond_signal(&runway_available_cond);
}

void *takeoff(void *p)
{
    int plane_id = *((int *)p);
    pthread_mutex_lock(&runway_mtx);

    if (!is_runway_available)
    {
        printf("Samolot o ID %d czeka na pas do startu\n", plane_id);
        pthread_cond_wait(&runway_available_cond, &runway_mtx);
    }
    is_runway_available = false;
    printf("Start samolotu o ID %d\n", plane_id);
    sleepRandomMilliseconds(500, 2000);
    airport_num--;
    takeoff_waiting_num--;
    is_runway_available = true;
    if (airport_num < N)
    {
        pthread_cond_signal(&space_available);
    }
    pthread_cond_signal(&runway_available_cond);
    printf("Liczba samolotow na lotnisku po starcie: %d\n", airport_num);
    pthread_mutex_unlock(&runway_mtx);
}

int main()
{
    pthread_attr_t tattr;

    struct sched_param param;
    if (pthread_attr_init(&tattr) != 0)
        perror("pthread_attr_init error");
    if (pthread_attr_getschedparam(&tattr, &param) != 0)
        perror("pthread_attr_getschedparam error");

    param.sched_priority = 0;

    while (1)
    {
        srand(time(NULL));
        sleep(1);
        int action = rand() % 2;
        if (action == 0)
        {
            // landing
            pthread_t landing_thread;

            int plane_id = rand() % 1000 + 1000;
            // random number from 1000 to 2000

            pthread_attr_setschedparam(&tattr, &param);
            // if (pthread_attr_setschedparam(&tattr, &param) != 0)
            // {
            //     perror("pthread_attr_setschedparam error for landing");
            // }

            if (pthread_create(&landing_thread, &tattr, landing, &plane_id) != 0)
            {
                perror("pthread_create error for landing");
            }
            pthread_detach(landing_thread);
        }
        else if (action == 1 && airport_num - takeoff_waiting_num > 0) // nie mozemy wystatowac samolotow ktorych nie mamy
        {
            printf("airport_num: %d\n", airport_num);
            // takeoff
            pthread_t takeoff_thread;
            // random number from 2000 to 3000
            int plane_id = rand() % 1000 + 2000;

            if (airport_num < K)
            {
                param.sched_priority = 1;
            }
            else
            {
                param.sched_priority = 0;
            }

            pthread_attr_setschedparam(&tattr, &param);
            // if (pthread_attr_setschedparam(&tattr, &param) != 0)
            // {
            //     perror("pthread_attr_setschedparam error for takeoff");
            // }
            takeoff_waiting_num++;
            if (pthread_create(&takeoff_thread, &tattr, takeoff, &plane_id) != 0)
            {
                perror("pthread_create error for takeoff");
            }
            pthread_detach(takeoff_thread);
        }
    }
}