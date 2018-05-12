#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <queue>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

// Global variables to track time and be shared/usable within threads
sem_t carSem;

// Initialize mutex variables
pthread_mutex_t flagPersonMutex;
pthread_cond_t flagPersonCondition;

// Keep track of number of cars that have been created
int carCounter = 0;

// Direction that incoming cars are arriving from
string currentDirection = "";

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

  if (pthread_mutex_init(&mutex, NULL)) {
    return -1;
  }

  if (pthread_cond_init(&conditionvar, NULL)) {
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

car createCar(char direction) {
  struct timespec arrival;
  struct car newCar;

  carCounter++;
  newCar.id = carCounter;
  newCar.direction = direction;
  arrival.tv_sec = (unsigned int)time(NULL);
  arrival.tv_nsec = 0;
  newCar.arrivalTime = arrival;

  return newCar;
}

void *produceNorth(void *args)
{
  cout <<"in north" << endl;

  while (1) {
    sem_wait(&carSem);
    pthread_mutex_lock(&flagPersonMutex);

    while ((rand() % 10) < 8) {
      cout << "in north locks" << endl;
      nReadyQ.push(createCar('N'));
    }

    cout << "north sleep 20" << endl;
    pthread_sleep(20);
    pthread_cond_signal(&flagPersonCondition);
    pthread_mutex_unlock(&flagPersonMutex);
    sem_post(&carSem);
  }
}

void *produceSouth(void *args)
{
  cout << "in south " << endl;

  while (1) {
    sem_wait(&carSem);
    pthread_mutex_lock(&flagPersonMutex);

    while ((rand() % 10) < 8) {
      cout << "in south locks" << endl;
      sReadyQ.push(createCar('N'));
    }

    cout << "south sleep 20" << endl;
    pthread_sleep(20);
    pthread_cond_signal(&flagPersonCondition);
    pthread_mutex_unlock(&flagPersonMutex);
    sem_post(&carSem);
  }
}

void switchDirection() {
  if (currentDirection == "north") {
    currentDirection == "south";
  } else {
    currentDirection == "north";
  }
  return;
}

void processCar() {
  struct car processedCar;
  ofstream carLog;

  if (currentDirection == "north") {
   processedCar = nReadyQ.front();
    nReadyQ.pop();
  } else {
   processedCar = sReadyQ.front();
    sReadyQ.pop();
  }

  cout << "Car removed from " << currentDirection << " queue. " << processedCar.id << endl;
  carLog.open("car.log");
  carLog << left << setw(12) << processedCar.id << processedCar.direction << processedCar.arrivalTime << "\n";
  carLog.close();

  return;
}

void workerSleep() {
  ofstream flagPersonLog;

  while (!nReadyQ.empty() && !sReadyQ.empty()) {
    cout << "process asleep @ " << time(NULL) << endl;

    flagPersonLog.open("flagperson.log");
    flagPersonLog << left << setw (12) << time(NULL) << "sleep\n";
    flagPersonLog.close();
    // pthread_sleep(1);
    pthread_cond_wait(&flagPersonCondition, &flagPersonMutex);
  }

  cout << "process awake @ " << time(NULL) << endl;
  flagPersonLog.open("flagperson.log");
  flagPersonLog << left << setw (12) << time(NULL) << "woken-up\n";
  flagPersonLog.close();
}

void *consume(void *args)
{
  while (1) {
    pthread_mutex_lock(&flagPersonMutex);

    cout << "consumer not waiting: " << currentDirection << endl;

    if (currentDirection == "north") {
      if (sReadyQ.size() >= 10) {
        switchDirection();
      } else if (nReadyQ.empty()) {
        workerSleep();
      } else {
        processCar();
      }
    }
    else {
      if (nReadyQ.size() >= 10) {
        switchDirection();
      } else if (sReadyQ.empty()) {
        workerSleep();
      } else {
        processCar();
      }
    }

    pthread_mutex_unlock(&flagPersonMutex);
  }

  return 0;
}


int main() {
  pthread_t sTid, nTid, t_id, fTid;
  int pshared = 1;
  int value = 1; //value is 1 because this is a lock
  srand(time(NULL));

  if (pthread_mutex_init(&flagPersonMutex, NULL)) {
    perror("mutex_init");
    return -1;
  }

  if (pthread_cond_init(&flagPersonCondition, NULL)) {
    perror("cond_init");
    return -1;
  }

  if (0 != sem_init(&carSem, pshared, value)) {
    perror("sem_init");
    return -1;
  }

  // Create thread for car producer in the North direction
  if (-1 == pthread_create(&nTid, NULL, produceNorth, NULL))
    return -1;

  // Create thread for car producer in the South direction
  if (-1 == pthread_create(&sTid, NULL, produceSouth, NULL))
    return -1;

  // Create thread for flag person consuming the cars
  if (-1 == pthread_create(&fTid, NULL, consume, NULL))
    return -1;

  while (1) {
    printf("Main Running\n");
    fflush(stdout);
    pthread_sleep(1);
  }

  sem_close(&carSem);
  pthread_mutex_destroy(&flagPersonMutex);
  pthread_cond_destroy(&flagPersonCondition);

  return 0;
}
