// C++ Hraders
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>


// Objects
#include "Aircraft.h"
#include "Limits.h"
#include "ComputerSystem.h"
#include "DataDisplay.h"
#include "Limits.h"
#include "PrimaryRadar.h"
#include "SecondaryRadar.h"
#include "Timer.h"
#include "AirTrafficControl.h"

/* Global Variables */
// Class Objects
std::vector<Aircraft *> planes; // vector of plane objects
PrimaryRadar *psr;
SecondaryRadar *ssr;
DataDisplay *display;
ComputerSystem *computerSystem;

// Data
time_t startTime;
time_t endTime;

// shm members
int shm_waitingPlanes;
void *waitingPtr;
int shm_flyingPlanes;
void *flyingPtr;
int shm_airspace;
void *airspacePtr;
int shm_period;
void *periodPtr;
int shm_display;
void *displayPtr;

// Mutex
pthread_mutex_t mutex;


int main() {
  AirTrafficControl atc;
  return 0;
}
