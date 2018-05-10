#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/******************************************************************************
pthread_sleep takes an integer number of seconds to pause the current thread We
provide this function because one does not exist in the standard pthreads library. We
simply use a function that has a timeout.
*****************************************************************************/
int pthread_sleep (int seconds) {
  pthread_mutex_t mutex;
  pthread_cond_t conditionvar;
  struct timespec timetoexpire;

  if (pthread_mutex_init(&mutex,NULL)) {
    return -1;
  }

  if(pthread_cond_init(&conditionvar,NULL)) {
    return -1;
  }

  //When to expire is an absolute time, so get the current time and add
  //it to our delay time
  timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;
  timetoexpire.tv_nsec = 0;
  return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
}

/******************************************************************************
* This is an example function that becomes a thread. It takes a pointer
* parameter so we could pass in an array or structure.
*****************************************************************************/
void *worker(void *arg) {
  while(1) {
    printf("Thread Running\n");
    fflush(stdout);
    pthread_sleep(1);
  }

  return NULL;
}

/******************************************************************************
* The main function is just an infinite loop that spawns off a second thread
* that also is an infinite loop. We should see two messages from the worker
* for every one from main.
*****************************************************************************/
int main() {
  pthread_t t_id;

  if ( -1 == pthread_create(&t_id, NULL, worker, NULL) ) {
    perror("pthread_create");
    return -1;
  }

  while(1) {
    printf("Main Running\n");
    fflush(stdout);
    pthread_sleep(2);
  }
  return 0;
}

// car = {
//   id,
//   direction,
//   arrivalTime, // when they appear on the
//   startTime,
//   endTime
// }
