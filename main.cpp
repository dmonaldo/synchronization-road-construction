#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include<cstring>
#include <queue>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include<sched.h>
using namespace std;

// Global variables to track time and be shared/usable within threads
sem_t carSem;

pthread_attr_t tat1,tat2,tat3;
int ret;
int prio1=70;
int prio2=6;
int prio3=1;
sched_param p1,p2,p3;

// Initialize mutex variables
pthread_mutex_t flagPersonMutex;
pthread_cond_t flagPersonCondition;

// Keep track of number of cars that have been created
int carCounter = 0;

// Direction that incoming cars are arriving from
char currentDirection[6];



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
// Car producer in the North direction
void *produceNorth(void *args) {
  cout <<"in north" << endl;
  struct timespec arrival;
  struct car newCar;

  while (1) {
    sem_wait(&carSem);
    pthread_mutex_lock(&flagPersonMutex);

    while ((rand() % 10) < 8) {

      if((rand() % 10) < 5 && (nReadyQ.size()<5))
      {
      cout << "Producing car in the north, LOCKS" << endl;
      carCounter++;
      newCar.id = carCounter;
      newCar.direction = 'N';
      arrival.tv_sec = (unsigned int)time(NULL);
      arrival.tv_nsec = 0;
      newCar.arrivalTime = arrival;
      nReadyQ.push(newCar);

     int k =nReadyQ.size();

     cout<<k<<endl;

      pthread_sleep(1);
    }
  
  else
  {
    cout << "Producing car in the south direction, MUTEX LOCKS" << endl;
      carCounter++;
      newCar.id = carCounter;
      newCar.direction = 'S';
      arrival.tv_sec = (unsigned int)time(NULL);
      arrival.tv_nsec = 0;
      newCar.arrivalTime = arrival;
      sReadyQ.push(newCar);
        int s=sReadyQ.size();

        cout<<s<<endl;
      pthread_sleep(1);
  }
}
    cout << "sleep 20 N" << endl;
    pthread_sleep(20);
    pthread_cond_signal(&flagPersonCondition);
    pthread_mutex_unlock(&flagPersonMutex);
    cout<<"UNLOCKS"<<endl;

    sem_post(&carSem);
  }


  return 0;
}

/*

// Car producer thread in the South direction
void *produceSouth(void *args) {


  cout << "in south " << endl;
  struct timespec arrival;
  struct car newCar;

  while (1) {
    sem_wait(&carSem);
    pthread_mutex_lock(&flagPersonMutex);

    while ((rand() % 10) < 8) {
      cout << "Producing car in the south direction, MUTEX LOCKS" << endl;
      carCounter++;
      newCar.id = carCounter;
      newCar.direction = 'S';
      arrival.tv_sec = (unsigned int)time(NULL);
      arrival.tv_nsec = 0;
      newCar.arrivalTime = arrival;
      sReadyQ.push(newCar);
      pthread_sleep(1);
    }

    cout << "sleep 20 S" << endl;
    pthread_sleep(5);
    pthread_cond_signal(&flagPersonCondition);
    pthread_mutex_unlock(&flagPersonMutex);
    cout<<"MUTEX UNLOCKS"<<endl;
    sem_post(&carSem);


  }
  return 0;
}
*/

// Changes the direction of traffic that the flag person is allowing to pass
// through the construction zone
void switchDirection() {
  if (strcmp(currentDirection,"north")==0) {
    strcpy(currentDirection,"south");
    cout<<"Switching direction to south"<<endl;
    cout<<currentDirection<<endl;
  } else {

    strcpy(currentDirection,"north");
        cout<<"Switching direction to north"<<endl;
        cout<<currentDirection<<endl;

  }
  return;
}



// Removes a car from the queue
// Simulates a car passing through the construction zone
void processCar() {
  struct car processedCar;

  ofstream carLog;

  if (strcmp(currentDirection,"north")==0) {
    processedCar = nReadyQ.front();
    cout<<"processing car";
    pthread_sleep(1);
    nReadyQ.pop();


  } else

  {
    processedCar = sReadyQ.front();
    pthread_sleep(1);
    sReadyQ.pop();
  }

  cout << left << setw(12) << processedCar.id << processedCar.direction
       << endl;
  carLog.open("car.log", ios_base::app);
  carLog << left << setw(12) << processedCar.id << processedCar.direction
         << "\n";
  carLog.close();

  return;
}

// Determines whether the flag person should be awake or asleep
void workerSleep() {
  ofstream flagPersonLog;

  while (!nReadyQ.empty() && !sReadyQ.empty()) {
    cout << left << setw (12) << time(NULL) << "sleep" << endl;

    flagPersonLog.open("flagperson.log", ios_base::app);
    flagPersonLog << left << setw (12) << time(NULL) << "sleep\n";
    flagPersonLog.close();
    pthread_sleep(1);
    pthread_cond_wait(&flagPersonCondition, &flagPersonMutex);
  }

  cout << left << setw (12) << time(NULL) << "woken-up" << endl;
  flagPersonLog.open("flagperson.log", ios_base::app);
  flagPersonLog << left << setw (12) << time(NULL) << "woken-up\n";
  flagPersonLog.close();
}

// Car consumer thread
void *consume(void *args) {
  while (1) {

    cout<<"hi, entered consume"<<endl;
    pthread_mutex_lock(&flagPersonMutex); //0 if he is awake.. consume

    cout << "consumer not waiting: " << currentDirection << endl;

    if (strcmp(currentDirection,"north")==0) {

      cout<<"direction is north"<<endl;
      if ((sReadyQ.size() >= 5) && (nReadyQ.size() < 5)) {

        cout<<"ready queue has more than 5 cars in the south";
        switchDirection();
      } else if (nReadyQ.empty() && sReadyQ.size() >= 5) {
                cout<<"here 1"<<endl;
        switchDirection();
      } else if (nReadyQ.empty()) {
                cout<<"here 2"<<endl;

        workerSleep();
      }
       else {

        cout<<"here 3"<<endl;
        processCar();
      }
    }
    else {

      cout<<"direction is south "<<endl;
      if (nReadyQ.size() >= 5 && sReadyQ.size() < 5) {

        cout<<"ready queue has more than 5 cars in the north";
        switchDirection();
      } else if (sReadyQ.empty() && nReadyQ.size() >= 5) {

        cout<<"i am here"<<endl;
        switchDirection();
      } else if (sReadyQ.empty()) {
        cout<<"hello"<<endl;
        workerSleep();
      } else {

        cout<<"processing south car"<<endl;
        processCar();
      }
    }


    pthread_mutex_unlock(&flagPersonMutex);
  }

  return 0;
}

/******************************************************************************
* The main function is just an infinite loop that spawns off a second thread
* that also is an infinite loop. We should see two messages from the worker
* for every one from main.
*****************************************************************************/
int main() {

  strcpy(currentDirection,"north");
  pthread_t sTid, nTid, fTid;
  int pshared = 1;
  int semValue = 1; //value is 1 because this is a lock
  srand(time(NULL));

  // Begin log files
  ofstream carLogInitial, flagPersonLogInitial;
  carLogInitial.open("car.log");
  carLogInitial << left << setw(12) << "carID" << "direction" << "arrival-time" << "start-time" << "end-time\n";
  carLogInitial.close();
  flagPersonLogInitial.open("flagperson.log");
  flagPersonLogInitial << left << setw (12) << "time" << "state\n";
  flagPersonLogInitial.close();

  // Initialize mutex
  if (pthread_mutex_init(&flagPersonMutex, NULL)) {
    perror("mutex_init");
    return -1;
  }

  // Initialize conditional variable
  if (pthread_cond_init(&flagPersonCondition, NULL)) {
    perror("cond_init");
    return -1;
  }

  // Initialize semaphore
  if (0 != sem_init(&carSem, pshared, semValue)) {
    perror("sem_init");
    return -1;
  }

ret=pthread_attr_init(&tat1);
ret=pthread_attr_getschedparam(&tat1,&p1);
p1.sched_priority=prio1;

ret=pthread_attr_init(&tat2);
ret=pthread_attr_getschedparam(&tat1,&p2);
p2.sched_priority=prio2;

ret=pthread_attr_init(&tat3);
ret=pthread_attr_getschedparam(&tat1,&p3);
p3.sched_priority=prio3;


  // Create thread for flag person consuming the cars
pthread_create(&fTid, &tat1, consume, NULL);

  // Create thread for car producer in the South direction
//pthread_create(&sTid, &tat2, produceSouth, NULL);


  // Create thread for car producer in the North direction
pthread_create(&nTid, &tat3, produceNorth, NULL);



  while (1) {
    fflush(stdout);
    pthread_sleep(1);
  }

  sem_close(&carSem);
  pthread_mutex_destroy(&flagPersonMutex);
  pthread_cond_destroy(&flagPersonCondition);

  return 0;
}
