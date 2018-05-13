# synchronization-road-construction
<<<<<<< HEAD
Models a common roadway occurrence, where a lane is closed and a flag person 
is directing traffic.
=======
Models a common roadway occurrence, where a lane is closed and a flag person is directing traffic.
>>>>>>> 93d36c6f87e2178e614dde9b328bd0f99db4c595

There is one lane closed of a two-lane road, with traffic coming from the North and South. Because of traffic lights, the traffic on the road comes in bursts. When a car arrives, there is an 80% chance of another car following it, but once no car comes, there is a 20 second delay (use the provided pthread_sleep here) before any new car will come.

<<<<<<< HEAD
Team member #2: Alex Runciman
Contributions: produceNorth, produceS, consume
Percentage of contributions:100% of produceNorth and produceSouth, 
designed consume entirely (except for locks) and a few lines wrote in consume
=======
During the times when no cars are at either end, the flag person will fall asleep. When a car arrives at either end, the flag person will wake up and allow traffic from that side to pass the construction area, until there are no more cars from that side, or until there are 10 cars or more lining up on the opposite side. If there are 10 cars or more on the opposite side, the flag person needs to allow the cars from the opposite side to pass. Each car takes 1 second to go through the construction area.

Your job is to construct a simulation of these events where under no conditions will a deadlock occur. A deadlock could either be that the flag person does not allow traffic through from either side, or let’s traffic through from both sides causing an accident.
>>>>>>> 93d36c6f87e2178e614dde9b328bd0f99db4c595

##Team members and contributions
Team member: Alex Runciman (@agrsu)
Contributions: producer functions
Percentage of contributions:

Team member: Dante Monaldo (@dmonaldo)
Contributions: consumer functions
Percentage of contributions:

<<<<<<< HEAD
/////// Thread information /////////////////////////////////////////
=======
##Thread information
>>>>>>> 93d36c6f87e2178e614dde9b328bd0f99db4c595
Total threads: 3

[Thread #1]
  --> Task: producer that produces cars in the North direction
  --> Thread function name: produceNorth()

[Thread #2]
  --> Task: producer that produces cars in the South direction
  --> Thread function name: produceSouth()

[Thread #3]
 --> Task: consumer that allows cars to pass through the construction zone
 --> Thread function name: consume()

<<<<<<< HEAD

////// Semaphores ////////////////////////////////////////////////
=======
##Semaphores
>>>>>>> 93d36c6f87e2178e614dde9b328bd0f99db4c595
Number of semaphores: 1

[Sempahore #1]
  --> Variable: carSem
  --> Initial value: 1
  --> Purpose: make xxx blocked upon on events xxxx

<<<<<<< HEAD
............



////// Mutex lock ///////////////////////////////////////////////
Number of mutex locks: 
=======
##Mutex lock
Number of mutex locks: 1
>>>>>>> 93d36c6f87e2178e614dde9b328bd0f99db4c595

[Mutex lock #1]
  --> Variable: flagPersonMutex
  --> Purpose: avoid race condition on shared data structure xxxx

##Strengths









##Weaknesses
