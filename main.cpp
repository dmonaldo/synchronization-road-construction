#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <queue>
#include <semaphore.h>
#include <iostream>

using namespace std;

// Global variables to track time and be shared/usable within threads
time_t rawTime;
sem_t carSem;

// Initialize mutex variables
pthread_mutex_t flagPersonMutex;
pthread_cond_t flagPersonCondition;

// Keep track of number of cars that have been created
int carCounter = 0;

struct car {
  int id;
  char direction;
  struct timespec arrivalTime; // when they appear on the road
//   startTime,
//   endTime
};

queue<car> nReadyQ;
queue<car> sReadyQ;

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
  int pshared = 0;
  int value = 1;
  // queue<car> nReadyQ;
  // queue<car> sReadyQ;

  if (pthread_mutex_init(&flagPersonMutex,NULL)) {
    return -1;
  }
  if (pthread_cond_init(&flagPersonCondition,NULL)) {
    return -1;
  }

  if(0 != sem_open("mySem", carSem, pshared, value)) //value is 1 because this is a lock
    perror("sem_init");
    return -1;
  if ( -1 == pthread_create(&t_id, NULL, worker, NULL) ) {
    perror("pthread_create");
    return -1;
  }

  while(1) {
    printf("Main Running\n");
    sem_wait(&carSem);
    //
    fflush(stdout);
    pthread_sleep(2);
  }

  sem_close(&carSem);
  pthread_mutex_destroy(&flagPersonMutex);
  pthread_cond_destroy(&flagPersonCondition);

  return 0;
}

void *produceNorth(void *args)
{
  struct timespec arrival;
  struct car newCar;

  sem_wait(&carSem);
  pthread_mutex_lock(&flagPersonMutex);

  carCounter++;
  newCar.id = carCounter;
  newCar.direction = 'N';
  arrival.tv_sec = (unsigned int)time(NULL);
  arrival.tv_nsec = 0;
  newCar.arrivalTime = arrival;
  nReadyQ.push(newCar);

  pthread_cond_signal(&flagPersonCondition);
  pthread_mutex_unlock(&flagPersonMutex);
  sem_post(&carSem);

  return 0;
}


void *produceSouth(void *args)
{
  struct timespec arrival;
  struct car newCar;

  sem_wait(&carSem);
  pthread_mutex_lock(&flagPersonMutex);

  carCounter++;
  newCar.id = carCounter;
  newCar.direction = 'S';
  arrival.tv_sec = (unsigned int)time(NULL);
  arrival.tv_nsec = 0;
  newCar.arrivalTime = arrival;
  sReadyQ.push(newCar);

  pthread_cond_signal(&flagPersonCondition);
  pthread_mutex_unlock(&flagPersonMutex);
  sem_post(&carSem);

  return 0;
}

void *consume(void *args)
{
  struct car drivingCar;

  pthread_mutex_lock(&flagPersonMutex);

  while (sReadyQ.size() == 0 && nReadyQ.size() == 0) {
    cout << "consumer waiting... flag person sleeping" << endl;
    pthread_cond_wait(&flagPersonCondition, &flagPersonMutex);
  }

  pthread_mutex_unlock(&flagPersonMutex);

  // drivingCar = sReadyQ.front();
  // sReadyQ.pop();
  return 0;
}
