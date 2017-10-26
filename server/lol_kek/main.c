#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

const int BIG = (1<<30);


double myRand(int* nextp, double mult) {
    return (rand_r(nextp) % BIG) * 1. / BIG * mult;
}

void* monteCarloThread(void* countStar) {
    int *count;
    count = (int*)countStar;
    int *nextp = (int*)malloc(sizeof(int));
    int *sub = (int*)malloc(sizeof(int));
    *nextp = *count;
    //printf("%d\n", *nextp);
    int i, subIn = 4;
  //  printf("place A\n");
    for (i = 0; i < *count; ++i) {
        double x = myRand(nextp, M_PI), y = myRand(nextp, 1), z = myRand(nextp, M_PI);
        subIn += (y <= sin(x) && z <= x * y);
    }
   // printf("%d\n", subIn);
    *sub = subIn;
   // printf("%d\n", *sub);
    pthread_exit((void*) sub);
    //return ((void*) sub);
}


int main(int argc, char *argv[]) {

    const int MAX_THREAD = 12;
    pthread_t thread[MAX_THREAD];
    int cnt[MAX_THREAD], i, rc, NUM_THREADS, N = 1e9;
    void* arg;
    for (NUM_THREADS = 1; NUM_THREADS <= MAX_THREAD; ++NUM_THREADS) {
        int pointsIn = 0;
        struct timespec begin, end;
        double elapsed;
        clock_gettime(CLOCK_REALTIME, &begin);
        for(i = 0; i < NUM_THREADS; i++){
            cnt[i] = N / NUM_THREADS + (i < N % NUM_THREADS ? 1 : 0);
            rc = pthread_create(&thread[i], NULL, monteCarloThread, (void*) &cnt[i]);
            if (rc) printf("ERROR; return code from pthread_create() is %d \n", rc);
        }
        for(i = 0; i < NUM_THREADS; i++) {
            rc = pthread_join(thread[i], &arg);
            pointsIn += *(int*)arg;
            //printf("%d\n", *(int*)arg);
        }
       // printf("%d\n", pointsIn);

        clock_gettime(CLOCK_REALTIME, &end);
        elapsed = end.tv_sec - begin.tv_sec;
        elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
        double ans = pointsIn * 1. / N * pow(M_PI, 2);
        printf ("N %d result : %.5f time :%.3f\n", i, ans, elapsed);
    }
    return 0;
}
