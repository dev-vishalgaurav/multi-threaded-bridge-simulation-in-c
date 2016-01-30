#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#define TO_NORWICH 1 // representing direction to Norwich
#define TO_HANOVER 2 // representing direction to Hanover
#define TO_NOWHERE 0 // representing direction to nothing :p

#define MAX_CARS 15 // maximum cars which can be used for simulation
#define MAX_CARS_BRIDGE 3 // maximum cars in single sirection on the bridge
#define MAX_CARS_BRIDGE_IN_A_ROW 5 // threshold for maximum cars in a row to a single direction  

#define true 1
#define false 0
#define place(x)  ( (TO_HANOVER == x) ? "Hanover" : "Norwich" )
#define indent(x)  ( (TO_HANOVER == x) ? "\t+++++++++++" : "\t###########" )
#define direction(random) ( (random % 2) == 0) ? TO_HANOVER  : TO_NORWICH ;
#define bridge_indent "**********"
#define bridge_status_indent "-----------"

/*
* Defininng structure for a Car represntaion. It consists of a pthread, direction and its id i.e number
*/
typedef struct{
  int number;
  int direction;
  pthread_t thread;
}Car;

/*
* Bridge structure direction = current movement in the bridge
* number = total cars for a direction in bridge
* total = consecutive cars in a row for a single direction on bridge
* lock = bridge lock
* waitingForHanover = number of cars in queue to Hanover
* waitingForNorwich = number of cars in queue to Norwich
* cvar_direction = condition variable for the bridge
*/
typedef struct{
  int direction ; // direction towards which current cars on the beridge are moving to 
  int number;
  int total;
  int waitingForHanover;
  int waitingForNorwich;
  pthread_mutex_t lock ; // required lock for mutual exclusion
  pthread_cond_t  cvar_direction;
}Bridge;

/*
* constant defining Empty Structure for a car used for resetting the value
*/
static const Car EmptyStruct;

/*
* default bridge state. initializing the bridge to default state
*/

Bridge bridge = {
  .number = 0,
  .direction = TO_NOWHERE,
  .total = 0,
  .waitingForHanover = 0,
  .waitingForNorwich = 0,
  .lock = PTHREAD_MUTEX_INITIALIZER,
  .cvar_direction = PTHREAD_COND_INITIALIZER,
};

/*
* major hooks (critical sections for this bridge simulaation)
*/
void *OneVehicle(void* car); // If it is called it means car has arrived to the bridege and may wait for its chance to get on the bridge
void ArriveBridge(Car *car);
void OnBridge(Car *car);
void ExitBridge(Car *car);
/*
* declaration ends for core simulation methods
*/

/**
* function declarations for bridge simulation program
*/
void showTutorial();
int getRequiredInputs();
void callEnterToContinue(void);
void createCarThreadsToHanover();
void createCarThreadsToNorwich();
void waitForCarThreadsToFinish();
int isRepeatProgram();
void doExitProcedure();
void resetValues();
int getNewDirection();
void assignRandomDirection();
Car carThreadsToHanover[MAX_CARS] ;
Car carThreadsToNorwich[MAX_CARS] ;


int carsToNorwich = 0 ; // maintaining count of how many cars are there for Norwich
int carsToHanover = 0 ; // maintaining count of how many cars are there for Hanover


int currentCarsOnBridge = 0 ;
int currentDirection = TO_NOWHERE;

/**
* start of the program. It starts with making sure inputs are proper for simulation
* if input is proper then it creates all the car threads and wait for them to finish
* after all cars are done it will ask user to repeat the process
* Note : Conditions like maximum car in bridge, threshold for continuous movement in single direction and max cars for simulation
*        can be changed by chaning these 
*         #define MAX_CARS 15
*         #define MAX_CARS_BRIDGE 3
*         #define MAX_CARS_BRIDGE_IN_A_ROW 5
**/
int main(int argc, char const *argv[]) {
  printf("Welcome to bridge simulation program \n");
  do{
    resetValues();
    if(getRequiredInputs()){
      showTutorial();
      callEnterToContinue();
      assignRandomDirection();
      createCarThreadsToHanover();
      createCarThreadsToNorwich();
      waitForCarThreadsToFinish();
    }else{
      printf("inputs are mandatory \n");
    }
  }while(isRepeatProgram());
  doExitProcedure();
  return 0;
}
/*Main program ends */
void assignRandomDirection(){
  bridge.direction = getNewDirection();
  printf("direction is %s\n",place(bridge.direction) );
}
/**
* Create all the threads for simulating cars going to Hanover
*/
void createCarThreadsToHanover(){
  int carsCount = 0 ;
  for (carsCount = 0; carsCount < carsToHanover; carsCount++) {
    carThreadsToHanover[carsCount].number = carsCount;
    carThreadsToHanover[carsCount].direction = TO_HANOVER ;
    pthread_create(&carThreadsToHanover[carsCount].thread,NULL,OneVehicle,(void*)&carThreadsToHanover[carsCount]);
  }
}
/**
* Create all the threads for simulating cars going to Norwich
*/
void createCarThreadsToNorwich(){
  int carsCount = 0 ;
  for (carsCount = 0; carsCount < carsToNorwich; carsCount++) {
    carThreadsToNorwich[carsCount].number = carsCount;
    carThreadsToNorwich[carsCount].direction = TO_NORWICH;
    pthread_create(&carThreadsToNorwich[carsCount].thread,NULL,OneVehicle,(void*)&carThreadsToNorwich[carsCount]);
  }
}
/**
* this function will make sure that main program exits only when all the child thread are done 
*/
void waitForCarThreadsToFinish(){
  int carsCount = 0 ;
  for (carsCount = 0; carsCount < carsToHanover; carsCount++) {
    pthread_join(carThreadsToHanover[carsCount].thread, NULL);
  }
  for (carsCount = 0; carsCount < carsToNorwich; carsCount++) {
    pthread_join(carThreadsToNorwich[carsCount].thread, NULL);
  }

}
/**
* This method is called when the vehicle arrives and following things are performed here
* 1. Checks the direction towards which vehicle is going and parse the car structure
* 2. Call Arrive Bridge (it doesn't returns untill conditions are right )
* 3. Call OnBridge which will print status of the bridge and checks the Assert conditions
* 4. Exit the bridge prining car number and direction
* Note : after call to each method ArriveBridge, OnBridge, ExitBrdge method calls sleep to promote interleavings with other threads, 
*         using this sleep it can simulate 3 cars at a time at bidge
*/
void *OneVehicle(void* vargs) {
  Car *car ;
  car = (Car *)vargs;
  printf("%s OneVehicle car %d to %s \n",bridge_indent,car->number,place(car->direction));
  ArriveBridge(car);
  sleep(1); // calling interleavings
  OnBridge(car);
  sleep(1); // calling interleavings
  ExitBridge(car);
  sleep(1); // calling interleavings
  pthread_exit(NULL);
}
/**
* When a car arrives at the bridge it checks for following things to avoid race condition and accidents and starving
* 1. get the lock for the bridge
* 2. Check the direction of the car and the bridge
* 3. wait till car is aligned to the direction and limits of the bridge
* 4. if its satisfy the condition then change the values in the bridge and then release the lock.
* Note : Following conditions are checked on the bridge 
*   Condition 1   if car direction is same as current cars running on the bridge
*     Condition 2   if bridge has threshhold for containing this car
*       Condition 3   if allwoing this will not create starving for cars waiting for other direction   
* if anyone of condition mantioned above is true then car has to wait !! 
* 
* Note : using third condition prevents starvation  
*/
void ArriveBridge(Car *car){
  pthread_mutex_lock(&bridge.lock);
  if(TO_HANOVER == car->direction)
    bridge.waitingForHanover++;
  if(TO_NORWICH == car->direction)
      bridge.waitingForNorwich++;

/*   
* wait till the following conditions are satisfied 
* if car direction is same as current cars running on the bridge
*   if bridge has threshhold for containing this car
*     if allwoing this will not create starving for cars waiting for other direction   
*/
  while(bridge.direction != car->direction || bridge.number >= MAX_CARS_BRIDGE || bridge.total >= MAX_CARS_BRIDGE_IN_A_ROW){
    pthread_cond_wait(&bridge.cvar_direction, &bridge.lock);
  }
  printf("%s(%d) ArriveBridge car %d to %s \n",indent(car->direction),car->number,car->number,place(car->direction));
  // conditions are favorable for the cars to get on the bridge
  bridge.number++; //  register this car to bridge 
  bridge.total++;  // counter to check starvation condition,
  pthread_cond_broadcast(&bridge.cvar_direction);
  pthread_mutex_unlock(&bridge.lock);
}
/**
* simulate car's On bridge conditon also check for race condition and call assert on critical paramaeters
*/
void OnBridge(Car *car){
  pthread_mutex_lock(&bridge.lock);
  // lock was acquired to check assert condtions below
  assert((car->direction == bridge.direction));
  assert(bridge.number <= MAX_CARS_BRIDGE);
  assert(bridge.total <= MAX_CARS_BRIDGE_IN_A_ROW);
  printf("%s(%d) OnBridge car %d to %s \n",indent(car->direction),car->number,car->number,place(car->direction));
  printf("%s Bridge Status \n| Cars = %d. \t|\n| direction = %s \t|\n| cars in a row = %d. \t|\n%s\n",bridge_status_indent,bridge.number,place(bridge.direction),bridge.total,bridge_status_indent);
  // releaseing lock now
  pthread_mutex_unlock(&bridge.lock);
}
/**
* get the bridge lock delete cars entry from the bridge register
* this function will also need bridge lock to change the bridge counters/register
* 
*/
void ExitBridge(Car *car){
  pthread_mutex_lock(&bridge.lock);
  printf("%s(%d) ExitBridge car %d to %s \n",indent(car->direction),car->number,car->number,place(car->direction));
  if(TO_HANOVER == car->direction)
    bridge.waitingForHanover--;
  if(TO_NORWICH== car->direction)
      bridge.waitingForNorwich--;
  // change bridge status and decrease a number of car on the bridge  
  bridge.number--;
  // if bridge is empty then signal all the thread to get a lock of bridge.
  // if bridge is empty then ot will toggle the direction of the bridge if there are cars waiting on the other side
  if(bridge.number == 0){
    printf("0 car waitingForNorwich = %d, waitingForHanover = %d \n",bridge.waitingForNorwich,bridge.waitingForHanover);
    // condition for checking if any car is waiting for other direction ?? 
    // if waiting then only change the direction else continue to same direction. 
    //
    if(TO_HANOVER == bridge.direction && bridge.waitingForNorwich > 0 ){
      bridge.direction = TO_NORWICH ;
    }else if (TO_NORWICH == bridge.direction && bridge.waitingForHanover > 0){ //only change direction if someone is waiting in opposite direction
      bridge.direction = TO_HANOVER ;
    }
    bridge.total = 0 ; // reset total counter only if direction is changed
    printf("direction cahnged to %s \n", place(bridge.direction ));
  }
  // signals other thread to check their turn to enter the bridge. 
  pthread_cond_broadcast(&bridge.cvar_direction);
  pthread_mutex_unlock(&bridge.lock);
}

int getNewDirection(){
  srand(time(NULL));
  int random = rand();
  return direction(random);

}
/**
* resets prgram values to simulate the bridge again
*/
void resetValues(){
  int carSize = 0 ;
  int maxValue = (carsToHanover >= carsToNorwich) ? carsToHanover : carsToNorwich ;
  for (carSize = 0; carSize < MAX_CARS; carSize++) {
    carThreadsToHanover[carSize] = EmptyStruct;
    carThreadsToNorwich[carSize] = EmptyStruct;
  }
  carsToHanover = 0 ;
  carsToNorwich = 0 ;
}
/*
* helper method which takes input from the user if he/she want to repeat the program. 
* accordingly it will result true/false
*/
int isRepeatProgram(){
  int result = false;
  char response ;
  printf("Enter a y to repeat else to exit  \n");
  scanf("%c",&response);
  getchar();
  if('y' == response || 'Y' == response){
    result = true;
  }
  return result;
}
void doExitProcedure(){
  printf("Program exited\n");
  callEnterToContinue();
}
/**
* get required inputs for the bridge simulation to work
*/ 
int getRequiredInputs(){
  int status = false;
  printf("Enter number of cars to Hanover maxvalue(%d)  \n",MAX_CARS);
  scanf("%d", &carsToHanover );
  getchar();
  while(carsToHanover <= 0 || carsToHanover > MAX_CARS ){
    printf("wrong input Please enter cars between %d and %d)  \n",1,MAX_CARS);
    scanf("%d", &carsToHanover );
    getchar();
  }
  printf("Enter number of cars to Norwich maxvalue(%d) \n",MAX_CARS);
  scanf("%d", &carsToNorwich );
  getchar();
  while(carsToNorwich <= 0 || carsToNorwich > MAX_CARS ){
    printf("wrong input Please enter cars between %d and %d)  \n",1,MAX_CARS);
    scanf("%d", &carsToNorwich );
    getchar();
  }
  printf("Your cars to Hanover are %d and your cars to Norwich are %d \n",carsToHanover,carsToNorwich);
  status = true;
  return status;
}
/*
* showing initial tutoral for interpreting result
*/
void showTutorial(){
  printf("\n\n%s -> (denotes a car to Hanovr)\n",indent(TO_HANOVER));
  printf("%s -> (denotes a car to Norwich)\n",indent(TO_NORWICH));
  printf("\t%s -> (denotes status of the bridge)\n\n",bridge_status_indent );
}

/**
*helper method to ask user to press enter and continue
*/
void callEnterToContinue(void){
	printf("Please press enter to continue :");
		while (true){
			int c = getchar();
      if (c == '\n' || c == EOF)
				break;
			}
}
