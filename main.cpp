#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <queue>
#include <semaphore.h>
#include <iostream>

using namespace std;

// Global variables to track time and be shared/usable within threads
sem_t carSem;

// Initialize mutex variables
pthread_mutex_t flagPersonMutex;
pthread_cond_t flagPersonCondition;

// Keep track of number of cars that have been created
int carCounter = 0;

string flagPersonDirection = "";

struct car {
  int id;
  char direction;
  struct timespec arrivalTime; // when they appear on the road
  struct timespec startTime;
  struct timespec endTime;
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
* The main function is just an infinite loop that spawns off a second thread
* that also is an infinite loop. We should see two messages from the worker
* for every one from main.
*****************************************************************************/

void *produceNorth(void *args)
{
  struct timespec arrival;
  struct car newCar;
  cout <<"in north" << endl;
  while(1){
  sem_wait(&carSem);
  pthread_mutex_lock(&flagPersonMutex);
  while((rand()%10)<8){
    cout << "in north locks" << endl;
    carCounter++;
    newCar.id = carCounter;
    newCar.direction = 'N';
    arrival.tv_sec = (unsigned int)time(NULL);
    arrival.tv_nsec = 0;
    newCar.arrivalTime = arrival;
    nReadyQ.push(newCar);
  }
  pthread_sleep(20);

  pthread_cond_signal(&flagPersonCondition);

  pthread_mutex_unlock(&flagPersonMutex);
  sem_post(&carSem);
  }
  //} 

  //pthread_sleep(20);
  
  //  return 0;
}
void *produceSouth(void *args)
{
  struct timespec arrival;
  struct car newCar;
  //bool nextCar = true;
  cout << "in south " << endl;
  //  while(nextCar){
  //cout << "in south while " << endl;
  while(1){
  while((rand()%10)<8){
    sem_wait(&carSem);
    pthread_mutex_lock(&flagPersonMutex);
    cout << "in south locks" << endl;
    carCounter++;
    newCar.id = carCounter;
    newCar.direction = 'S';
    arrival.tv_sec = (unsigned int)time(NULL);
    arrival.tv_nsec = 0;
    newCar.arrivalTime = arrival;
    sReadyQ.push(newCar);
  }
  pthread_sleep(20);
  pthread_cond_signal(&flagPersonCondition);
  pthread_mutex_unlock(&flagPersonMutex);
  sem_post(&carSem);
  }    //}
  //if (rand()%11 < 8) {
  //  nextCar = true;
  //  pthread_sleep(1);
  //} else {
  //  nextCar = false;
  //pthread_sleep(20);
  
      //}
  
  //return 0;
}
void *consume(void *args)
{
  struct car drivingCar;

  pthread_mutex_lock(&flagPersonMutex);

  while (sReadyQ.size() == 0 || nReadyQ.size() == 0) {
    cout << "consumer waiting..." << endl;
    pthread_cond_wait(&flagPersonCondition, &flagPersonMutex);
  }

  cout << "consumer not waiting: " << flagPersonDirection << endl;
  //first check if north or south is above ten then its priority and if this is
  //not met then its arbitrary and  
  if (flagPersonDirection == "north") {
    if (sReadyQ.size() >= 10) {
      drivingCar = sReadyQ.front();
      sReadyQ.pop();
      flagPersonDirection = "south";
    } else {
      drivingCar = nReadyQ.front();
      nReadyQ.pop();
    }
  } else if (flagPersonDirection == "south") {

    if (nReadyQ.size() >= 10) {
      drivingCar = nReadyQ.front();
      nReadyQ.pop();
      flagPersonDirection = "north";
    } else {
      drivingCar = sReadyQ.front();
      sReadyQ.pop();
    }
  } else {

    if (sReadyQ.size() > nReadyQ.size()) {
      drivingCar = sReadyQ.front();
      sReadyQ.pop();
      flagPersonDirection = "south";
    } else {
      drivingCar = nReadyQ.front();
      nReadyQ.pop();
      flagPersonDirection = "north";
    }
  }

  pthread_mutex_unlock(&flagPersonMutex);

  // drivingCar = sReadyQ.front();
  // sReadyQ.pop();
  return 0;
}


int main() {
  pthread_t sTid, nTid, t_id, fTid;
  int pshared = 1;
  int value = 1; //value is 1 because this is a lock
  srand(time(NULL));
  if (pthread_mutex_init(&flagPersonMutex, NULL)) {
    return -1;
  }
  if (pthread_cond_init(&flagPersonCondition, NULL)) {
    return -1;
  }

  if (0 != sem_init(&carSem, pshared, value)) {
    perror("sem_init");
    return -1;
  }

  if (-1 == pthread_create(&nTid, NULL, produceNorth, NULL))
    return -1;

  if (-1 == pthread_create(&sTid, NULL, produceSouth, NULL))
    return -1;

  if(-1 == pthread_create(&fTid, NULL, consume, NULL))
    return -1;
  while (1) {
    printf("Main Running\n");
    fflush(stdout);
    pthread_sleep(1);
  }
  //pthread_join(produceNorth, NULL);
  //pthread_join(produceSouth, NULL);
  //  pthread_join(consume, NULL);
  sem_close(&carSem);
  pthread_mutex_destroy(&flagPersonMutex);
  pthread_cond_destroy(&flagPersonCondition);

  return 0;
}
/* 
void *consume(void *args)
{
  struct car drivingCar;

  pthread_mutex_lock(&flagPersonMutex);

  while (sReadyQ.size() == 0 || nReadyQ.size() == 0) {
    cout << "consumer waiting..." << endl;
    pthread_cond_wait(&flagPersonCondition, &flagPersonMutex);
  }

  cout << "consumer not waiting: " << flagPersonDirection << endl;

  if (flagPersonDirection == "north") {
    if (sReadyQ.size() >= 10) {
      drivingCar = sReadyQ.front();
      sReadyQ.pop();
      flagPersonDirection = "south";
    } else {
      drivingCar = nReadyQ.front();
      nReadyQ.pop();
    }
  } else if (flagPersonDirection == "south") {

    if (nReadyQ.size() >= 10) {
      drivingCar = nReadyQ.front();
      nReadyQ.pop();
      flagPersonDirection = "north";
    } else {
      drivingCar = sReadyQ.front();
      sReadyQ.pop();
    }
  } else {

    if (sReadyQ.size() > nReadyQ.size()) {
      drivingCar = sReadyQ.front();
      sReadyQ.pop();
      flagPersonDirection = "south";
    } else {
      drivingCar = nReadyQ.front();
      nReadyQ.pop();
      flagPersonDirection = "north";
    }
  }

  pthread_mutex_unlock(&flagPersonMutex);

  // drivingCar = sReadyQ.front();
  // sReadyQ.pop();
  return 0;
}
*/
