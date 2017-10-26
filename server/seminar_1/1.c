#include "stdlib.h"
#include "stdio.h"
#include "pthread.h"
#include "semaphore.h"

#define NUM_THREADS 2
sem_t sem;

int sum;
void* start_func(void* param) {
	int *local;
	int val;
	int i;
	/*local = (int*)param;*/
	
	/*sem_wait(&sem); *//* --sem; if == 0: wait ;; critical section start*/
	/*sem_post(&sem);	*//* ++sem ;; critical section end*/

	for (i = 0; i < 1000; ++i) {
		sum += i;
	} 
	/*sem_getvalue(&sem, &val);*/
/*
 * 	pthread_exit(NULL);*/
  	pthread_exit( (void*) &sum);
 	
	printf("thread!!!\n");
	return NULL;
}


int main(int argc, char *argv[]) {
	int param;
	int rc;
	void *arg;
	int i;
	pthread_t pthr[NUM_THREADS];
	
	sem_init(&sem, 0, 1); /* attribute, start value */
	
	sum = 0;
	for (i = 0; i < NUM_THREADS; ++i) {
		rc = pthread_create(&pthr[i], NULL, start_func, NULL);
		if (rc) {
			printf("Error creating thread %d\n", rc);
		}
	}
	/*rc = pthread_create(&pthr, NULL, start_func, (void*) &param);
	*/
	for (i = 0; i < NUM_THREADS; ++i) {
		rc = pthread_join(pthr[i], &arg);
		printf("value from func %d \n", *(int*)arg);
		
		if (rc)  {
			printf("Error join thread %d\n", rc);	
		}
	}
	/*pthread_join(pthr, &arg);	
	*/
	printf("Sum = %s \n", sum);
	printf("Hello, world!\n");
	sem_destroy(&sem);
	return 0;
}
