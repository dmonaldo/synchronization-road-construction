#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <queue>
#include <semaphore.h>

//global variables to track time and be shared/usable within threads
time_t rawTime;
sem_t carSem;
int carCounter = 0;
//queue<car> nReadyQ -- maybe need to decalre here?? same for sReadyQ
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

  queue<car> nReadyQ;

  sem_init(&carSem, 1, 0);

  if ( -1 == pthread_create(&t_id, NULL, worker, NULL) ) {
    perror("pthread_create");
    return -1;
  }

  while(1) {
    printf("Main Running\n");
    fflush(stdout);
    pthread_sleep(2);
  }

  sem_destroy(&carSem);

  return 0;
}

struct car{
  int id;
  char direction;
  struct timespec arrivalTime; // when they appear on the road
//   startTime,
//   endTime
};

void *produceNorth(void *args)
{
  struct timespec arrival;
  struct car newCar;
  carCounter++;
  newCar.id = carCounter;
  newCar.direction = 'N';
  arrival.tv_sec = (unsigned int)time(NULL);
  arrival.tv_nsec = 0;
  newCar.arrivalTime = arrival;
  nReadyQ.push(newCar);
}

void *produceSouth(void *args)
{
  struct timespec arrival;
  struct car newCar;
  carCounter++;
  newCar.id = carCounter;
  newCar.direction = 'S';
  arrival.tv_sec = (unsigned int)time(NULL);
  arrival.tv_nsec = 0;
  newCar.arrivalTime = arrival;
  sReadyQ.push(newCar);
}

void *consume(void *args)
{
  struct car drivingCar;

  sem_wait(carSem);
  drivingCar = sReadyQ.front();
  sReadyQ.pop();
  sem_post(carSem);

}
