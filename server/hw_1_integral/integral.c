#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#define __USE_MISC 1 /* M_PI */
#define MAX_THREADS 32
#include <math.h>
#include <tgmath.h>
#include <time.h>


const double MAX_X = M_PI;
const double MAX_Y = 1.;
const double MAX_Z = M_PI; /* MAX_X * MAX_Y */

int LOG;

double drand(double max, unsigned short xsubi[3]) {
    return erand48(xsubi) * max;
}

int accept(double x, double y, double z) {
    double f = x * y;
    return ((y <= sin(x)) && (z <= f));
}

void* evaluate_accepted_dots_3(void* args) {
    int total = *((int*)args);
    int dots = total;
    int accepted = 0;
    int *returned_value = malloc(sizeof(int));
    unsigned short xsubi[3]; /* thread-local */

    unsigned int rand_seed = (long long)args;
    xsubi[0] = rand_r(&rand_seed);
    xsubi[1] = rand_r(&rand_seed);
    xsubi[2] = rand_r(&rand_seed);

    if (LOG) {
        /** Here should be critical section, but ... print never brokes :) **/
        printf("New thread(Mk.3) (dots, xsubi[0]) = %d, %d\n", total, xsubi[0]);
    }

    while (dots-- > 0) {
        double x = drand(MAX_X, xsubi);
        double y = drand(MAX_Y, xsubi);
        double z = drand(MAX_Z, xsubi);

        if (accept(x, y, z)) {
            ++accepted;
        }

    }

    *returned_value = accepted;
    pthread_exit(returned_value);
}

double evaluate_integral_3(int threads_count, int dots_count) {
    pthread_t threads[MAX_THREADS];
    int dots_per_thread[MAX_THREADS];
    int rc;
    int* ret_ptr;
    int i;
    int accepted = 0;

    for (i = 0; i < threads_count; ++i) {
        if (i == 0) {
            dots_per_thread[i] = dots_count / threads_count +
                    dots_count % threads_count;
        } else {
            dots_per_thread[i] = dots_count / threads_count;
        }
        rc = pthread_create(&threads[i], NULL,
                evaluate_accepted_dots_3, (void*)(dots_per_thread + i));

        if (rc) {
            printf("Error creating thread: %d\n", rc);
            exit(1);
        }
    }

    for (i = 0; i < threads_count; ++i) {
        rc = pthread_join(threads[i], (void*)&(ret_ptr));
        if (rc) {
            printf("Error joining thread: %d\n", rc);
            exit(1);
        }
        accepted += *ret_ptr;
        if (LOG) {
            printf("Thread joined (accepted) = %d\n", accepted);
        }
        free(ret_ptr);
    }


    return MAX_X * MAX_Y * MAX_Z * (double) accepted / (double) dots_count;
}



/************************************/

int global_accepted = 0;
sem_t sem;

void* evaluate_accepted_dots_1(void* args) {
    int total = *((int*)args);
    int dots = total;
    int accepted = 0;
    unsigned short xsubi[3]; /* thread-local */

    unsigned int rand_seed = (long long)args;
    xsubi[0] = rand_r(&rand_seed);
    xsubi[1] = rand_r(&rand_seed);
    xsubi[2] = rand_r(&rand_seed);

    if (LOG) {
        sem_wait(&sem);
        printf("New thread(Mk.1) (dots, xsubi[0]) = %d, %d\n", total, xsubi[0]);
        sem_post(&sem);
    }

    while (dots-- > 0) {
        double x = drand(MAX_X, xsubi);
        double y = drand(MAX_Y, xsubi);
        double z = drand(MAX_Z, xsubi);

        if (accept(x, y, z)) {
            ++accepted;
        }

    }

    sem_wait(&sem);
    global_accepted += accepted;
    sem_post(&sem);
    return NULL;
}

double evaluate_integral_1(int threads_count, int dots_count) {
    pthread_t threads[MAX_THREADS];
    int dots_per_thread[MAX_THREADS];
    int rc;
    int i;
    global_accepted = 0;

    sem_init(&sem, 0, 1);

    for (i = 0; i < threads_count; ++i) {
        if (i == 0) {
            dots_per_thread[i] = dots_count / threads_count +
                    dots_count % threads_count;
        } else {
            dots_per_thread[i] = dots_count / threads_count;
        }
        rc = pthread_create(&threads[i], NULL,
                evaluate_accepted_dots_1, (void*)(dots_per_thread + i));

        if (rc) {
            printf("Error creating thread: %d\n", rc);
            exit(1);
        }
    }

    for (i = 0; i < threads_count; ++i) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            printf("Error joining thread: %d\n", rc);
            exit(1);
        }
        if (LOG) {
            printf("Thread joined (accepted) = %d\n", global_accepted);
        }
    }

    sem_destroy(&sem);

    return MAX_X * MAX_Y * MAX_Z *
            (double) global_accepted / (double) dots_count;
}

void test_drand(int MX) {
    unsigned short xsubi[3];
    double* a = malloc(sizeof(double) * MX);

    xsubi[0] = rand();
    xsubi[1] = rand();
    xsubi[2] = rand();

    int ten[10];
    double sum = 0.;
    int i;
    for (i = 0; i < 10; ++i) {
        ten[i] = 0;
    }
    for (i = 0; i < MX; ++i) {
        double val = drand(1., xsubi);
        sum += val;

        ++ten[(int)(val * 10)];
    }
    printf("Rates = ");
    for (i = 0; i < 10; ++i) {
        printf("%d, ", ten[i]);
    }
    printf("\nMean = %f\n", sum/MX);
    free(a);
}

void run(int total_threads_count, int total_dots_count, FILE* logfile) {
    struct timespec beg, end;
    double elapsed = 0.;
    double returned = 0.;

    clock_gettime(CLOCK_REALTIME, &beg);
    returned = evaluate_integral_1(total_threads_count, total_dots_count);
    clock_gettime(CLOCK_REALTIME, &end);

    elapsed = end.tv_sec - beg.tv_sec;
    elapsed += (end.tv_nsec - beg.tv_nsec) / 1000000000.0;
    printf("(threads, totaldots, sec, ret) = (%d, %d, %f, %f)\n",
           total_threads_count, total_dots_count, elapsed, returned);
    fprintf(logfile, "(%d, %d, %f, %f)\n",
            total_threads_count, total_dots_count, elapsed, returned);
}

int main(int argc, char *argv[])
{
    int total_dots_count = (int)1e7;
    int total_threads_count = 1;


    (void)argc;
    (void)argv;

    FILE* logfile = fopen("timelog.txt", "wt");
    if (!logfile) {
        printf("Log file open failed.Terminated.\n");
        exit(1);
    }
    fprintf(logfile, "[");
    if (argc <= 1) {
        int MAX_THREADS_RUN = 12;
        int ATTEMPTS = 3;
        int t = 1;
        LOG = 0;
        for (t = 1; t <= MAX_THREADS_RUN; ++t) {
            int j;
            for (j = 0; j < ATTEMPTS; ++j) {
                run(t, total_dots_count, logfile);
            }
        }
    } else {
        LOG = 1;
        total_threads_count = atoi(argv[1]);
        if (argc >=3 ) {
            total_dots_count = atoi(argv[2]);
        }
        run(total_threads_count, total_dots_count, logfile);
    }
    fprintf(logfile, "]");
    fclose(logfile);


    /* srand(42);
     * test_drand(1000);*/
    return 0;
}

