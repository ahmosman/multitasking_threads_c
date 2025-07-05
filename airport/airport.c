// On an aircraft carrier, planes land and take off. To do so, they need exclusive access to the runway.
// The carrier can accommodate a fixed number N of planes. If the number of planes on the carrier is less
// than K (K < N), then landing planes have priority for the runway.

// Task objective: synchronization of planes (runway and space on the carrier are resources)

// Source: Weiss Z., Gruźlewski T., Concurrent and Distributed Programming in Examples and Exercises. WNT Warsaw 1993

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
        printf("Plane with ID %d is waiting in the landing queue\n", plane_id);
        pthread_cond_wait(&space_available, &space_mtx);
    }
    pthread_mutex_unlock(&space_mtx);

    printf("Plane with ID %d is waiting for the runway to land\n", plane_id);

    pthread_mutex_lock(&runway_mtx);
    printf("Landing plane with ID %d\n", plane_id);
    landing_waiting_num--;

    sleepRandomMilliseconds(1000, 3000);

    airport_num++;

    printf("Number of planes at the airport after landing plane with ID %d: %d\n", plane_id, airport_num);
    pthread_mutex_unlock(&runway_mtx);
    pthread_cond_signal(&runway_available_cond);
}

void *takeoff(void *p)
{
    int plane_id = *((int *)p);
    printf("Plane with ID %d is waiting for the runway to take off\n", plane_id);

    pthread_mutex_lock(&runway_mtx);

    printf("Plane with ID %d is taking off\n", plane_id);
    sleepRandomMilliseconds(500, 2000);
    airport_num--;
    takeoff_waiting_num--;
    if (airport_num < N)
    {
        pthread_cond_signal(&space_available);
    }
    pthread_cond_signal(&runway_available_cond);
    printf("Number of planes at the airport after takeoff of plane with ID %d: %d\n", plane_id, airport_num);
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

            pthread_attr_setschedparam(&tattr, &param);

            if (pthread_create(&landing_thread, &tattr, landing, &plane_id) != 0)
            {
                perror("pthread_create error for landing");
            }
            pthread_detach(landing_thread);
        }
        else if (action == 1 && airport_num - takeoff_waiting_num > 0) // we can't take off planes we don't have
        {
            printf("airport_num: %d\n", airport_num);
            // takeoff
            pthread_t takeoff_thread;
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

            takeoff_waiting_num++;
            if (pthread_create(&takeoff_thread, &tattr, takeoff, &plane_id) != 0)
            {
                perror("pthread_create error for takeoff");
            }
            pthread_detach(takeoff_thread);
        }
    }
}